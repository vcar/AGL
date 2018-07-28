#define  _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <fcntl.h>

#include <json-c/json.h>

#include <afb/afb-binding.h>

struct signal {
	const char *name;
	struct afb_event event;
};

static const struct afb_binding_interface *afbitf;
static int playing;
static int stoping;

static struct signal signals[] = {
	{ .name = "START" },
	{ .name = "STOP" },
	{ .name = "Long_Term_Fuel_Trim_Bank1" },
	{ .name = "Intake_air_pressure" },
	{ .name = "Accelerator_Pedal_value" },
	{ .name = "Fuel_consumption" },
	{ .name = "Torque_of_friction" },
	{ .name = "Maximum_indicated_engine_torque" },
	{ .name = "Engine_torque" },
	{ .name = "Calculated_LOAD_value" },
	{ .name = "Activation_of_Air_compressor" },
	{ .name = "Engine_coolant_temperature" },
	{ .name = "Transmission_oil_temperature" },
	{ .name = "Wheel_velocity_front_left-hand" },
	{ .name = "Wheel_velocity_front_right-hand" },
	{ .name = "Wheel_velocity_rear_left-hand" },
	{ .name = "Torque_converter_speed" }
};

static struct signal *getsig(const char *name)
{
	int low, hig, mid, cmp;

	low = 0;
	hig = sizeof signals / sizeof * signals;
	while (low < hig) {
		mid = (low + hig) >> 1;
		cmp = strcmp(signals[mid].name, name);
		if (cmp == 0) {
			return &signals[mid];
		} else {
			if (cmp > 0)
				hig = mid;
			else
				low = mid + 1;
		}
	}
	return NULL;
}


static void send_trace(const char *name, struct json_object *object)
{
	struct signal *sig = getsig(name);

	if (sig && afb_event_is_valid(sig->event))
		afb_event_push(sig->event, json_object_get(object));
}

static void *play_traces(void *opaque)
{
	int len, fd;
	struct json_object *args = opaque;
	struct json_tokener *tokener = NULL;
	struct json_object *object;
	char line[1024];
	FILE *file = NULL;
	const char *info;
	const char *name;
	double speed = 1.0;
	int started = 0;
	double init = 0.0;
	double prev = 0.0;
	double base = 0.0;
	struct json_object *ots;
	struct json_object *on;
	double t, i, f;
	int hasots;
	int hason;
	struct timespec ts;

	/* creates the tokener */
	tokener = json_tokener_new();
	if (tokener == NULL) {
		info = "can't allocate tokener";
		goto end;
	}

	/* get the speed */
	if (json_object_object_get_ex(args, "speed", &object)) {
		speed = json_object_get_double(object);
		speed = speed <= 0 ? 1.0 : 1.0 / speed;
	}

	/* open the file */
	if (!json_object_object_get_ex(args, "filename", &object)) {
		info = "can't find filename";
		goto end;
	}
	fd = afb_daemon_rootdir_open_locale(afbitf->daemon, json_object_get_string(object), O_RDONLY, NULL);
	if (fd < 0) {
		info = "can't open the file";
		goto end;
	}
	file = fdopen(fd, "r");
	if (file == NULL) {
		close(fd);
		info = "can't instanciate the file";
		goto end;
	}

	/* send the start signal */
	send_trace("START", NULL);

	/* reads the file and send its content */
	while(!stoping && fgets(line, (int)sizeof line, file)) {
		len = (int)strlen(line);
		if (len && line[len-1] == '\n')
			len--;
		if (len) {
			json_tokener_reset(tokener);
			object = json_tokener_parse_ex(tokener, line, len);
			/* silently ignore errors ?!! */
			if (object != NULL) {
				hason = json_object_object_get_ex(object, "name", &on);
				hasots = json_object_object_get_ex(object, "timestamp", &ots);
				if (hasots && hason) {
					if (started)
						t = speed * (json_object_get_double(ots) - init);
					else {
						init = json_object_get_double(ots);
						started = 1;
						clock_gettime(CLOCK_REALTIME, &ts);
						base = (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
						t = 0;
					}
					json_object_object_add(object, "timestamp", json_object_new_double(t));

					if (t > prev) {
						f = modf(base + t, &i);
						ts.tv_sec = (time_t)i;
						ts.tv_nsec = (long)(1000000000.0 * f);
						prev = t;
						clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);
					}

					send_trace(json_object_get_string(on), object);
				}
				json_object_put(object);
			}
		}
	}
	info = NULL;

end:
	/* send the stop signal */
	send_trace("STOP", NULL);

	/* cleanup */
	if (file)
		fclose(file);
	if (tokener)
		json_tokener_free(tokener);
	json_object_put(args);

	/* terminate */
	stoping = 0;
	playing = 0;
	return NULL;
}


static void start(struct afb_req request)
{
	struct json_object *args, *a;
	int fd;
	pthread_t tid;

	/* check filename argument */
	args = afb_req_json(request);
	if (!json_object_object_get_ex(args, "filename", &a)) {
		afb_req_fail(request, "error", "argument 'filename' is missing");
		return;
	}
	fd = afb_daemon_rootdir_open_locale(afbitf->daemon, json_object_get_string(a), O_RDONLY, NULL);
	if (fd < 0) {
		afb_req_fail(request, "error", "argument 'filename' is not a readable file");
		return;
	}
	close(fd);

	/* check speed argument */
	if (json_object_object_get_ex(args, "speed", &a)) {
		if (json_object_get_double(a) <= 0) {
			afb_req_fail(request, "error", "argument 'speed' is not a valid positive number");
			return;
		}
	}

	/* check the state */
	if (playing) {
		afb_req_fail(request, "error", "already playing");
		return;
	}

	/* valid then try to start */
	playing = 1;
	stoping = 0;
	if (pthread_create(&tid, NULL, play_traces, json_object_get(args)) != 0) {
		playing = 0;
		afb_req_fail(request, "error", "can't start to play");
		return;
	}

	afb_req_success(request, NULL, NULL);
}

static void stop(struct afb_req request)
{
	if (playing)
		stoping = 1;
	afb_req_success(request, NULL, NULL);
}

static int subscribe_unsubscribe_sig(struct afb_req request, int subscribe, struct signal *sig)
{
	if (!afb_event_is_valid(sig->event)) {
		if (!subscribe)
			return 1;
		sig->event = afb_daemon_make_event(afbitf->daemon, sig->name);
		if (!afb_event_is_valid(sig->event)) {
			return 0;
		}
	}

	if (((subscribe ? afb_req_subscribe : afb_req_unsubscribe)(request, sig->event)) < 0) {
		return 0;
	}

	return 1;
}

static int subscribe_unsubscribe_all(struct afb_req request, int subscribe)
{
	int i, n, e;

	n = sizeof signals / sizeof * signals;
	e = 0;
	for (i = 0 ; i < n ; i++)
		e += !subscribe_unsubscribe_sig(request, subscribe, &signals[i]);
	return e == 0;
}

static int subscribe_unsubscribe_name(struct afb_req request, int subscribe, const char *name)
{
	struct signal *sig;

	if (0 == strcmp(name, "*"))
		return subscribe_unsubscribe_all(request, subscribe);

	sig = getsig(name);
	if (sig == NULL) {
		return 0;
	}

	return subscribe_unsubscribe_sig(request, subscribe, sig);
}

static void subscribe_unsubscribe(struct afb_req request, int subscribe)
{
	int ok, i, n;
	struct json_object *args, *a, *x;

	/* makes the subscription/unsubscription */
	args = afb_req_json(request);
	if (args == NULL || !json_object_object_get_ex(args, "event", &a)) {
		ok = subscribe_unsubscribe_all(request, subscribe);
	} else if (json_object_get_type(a) != json_type_array) {
		ok = subscribe_unsubscribe_name(request, subscribe, json_object_get_string(a));
	} else {
		n = json_object_array_length(a);
		ok = 0;
		for (i = 0 ; i < n ; i++) {
			x = json_object_array_get_idx(a, i);
			if (subscribe_unsubscribe_name(request, subscribe, json_object_get_string(x)))
				ok++;
		}
		ok = (ok == n);
	}

	/* send the report */
	if (ok)
		afb_req_success(request, NULL, NULL);
	else
		afb_req_fail(request, "error", NULL);
}

static void subscribe(struct afb_req request)
{
	subscribe_unsubscribe(request, 1);
}

static void unsubscribe(struct afb_req request)
{
	subscribe_unsubscribe(request, 0);
}

// NOTE: this sample does not use session to keep test a basic as possible
//       in real application most APIs should be protected with AFB_SESSION_CHECK
static const struct afb_verb_desc_v1 verbs[]= {
  {"start",      AFB_SESSION_CHECK, start       , "start to play a trace"},
  {"stop",       AFB_SESSION_CHECK, stop        , "stop to play a trace"},
  {"subscribe",  AFB_SESSION_CHECK, subscribe   , "subscribes to the event of 'name'"},
  {"unsubscribe",AFB_SESSION_CHECK, unsubscribe , "unsubscribes to the event of 'name'"},
  {NULL}
};

static const struct afb_binding plugin_desc = {
	.type = AFB_BINDING_VERSION_1,
	.v1 = {
		.info = "trace openXC service",
		.prefix = "txc",
		.verbs = verbs
	}
};

const struct afb_binding *afbBindingV1Register (const struct afb_binding_interface *itf)
{
	afbitf = itf;
	return &plugin_desc;
}

