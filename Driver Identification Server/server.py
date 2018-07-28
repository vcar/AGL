from flask import Flask, render_template, request, jsonify
from flask_cors import CORS

import numpy as np
from sklearn import preprocessing
from sklearn.externals import joblib

from keras.models import Sequential
from keras.layers import Dense
from keras.layers import *

from keras import optimizers

from keras.models import Model
from keras.layers import Input, PReLU, Dense, LSTM, multiply, concatenate, Activation
from keras.layers import Conv1D, BatchNormalization, GlobalAveragePooling1D, Permute, Dropout

def generate_model():
    #ip = Input(shape=(valid_data.shape[1], valid_data.shape[2]))
    ip = Input(shape=(data.shape[1], data.shape[2]))
    x = Permute((2, 1))(ip)
    x=LSTM(10)(x)
    x = Dropout(0.8)(x)
    y = Conv1D(128, 8, padding='same', kernel_initializer='he_uniform')(ip)
    y = BatchNormalization()(y)
    y = Activation('relu')(y)
    y = Conv1D(256, 5, padding='same', kernel_initializer='he_uniform')(y)
    y = BatchNormalization()(y)
    y = Activation('relu')(y)
    y = Conv1D(128, 3, padding='same', kernel_initializer='he_uniform')(y)
    y = BatchNormalization()(y)
    y = Activation('relu')(y)
    y = GlobalAveragePooling1D()(y)
    x = concatenate([x, y])
    out = Dense(10, activation='sigmoid')(y)
    model = Model(ip, out)
    #model.summary()

    # add load model code here to fine-tune
    return model

app = Flask(__name__)
CORS(app)

aa=bb=cc=dd=ee=ff=gg=hh=ii=jj=kk=ll=mm=nn=oo=[]

def listo(sisi,si):
	if len(sisi)<60:
		sisi.append(si)
	return sisi

def do_predictions():
	if(len(aa)==len(bb)==len(cc)==len(dd)==len(ee)==len(ff)==len(gg)==len(hh)==len(ii)==len(jj)==len(kk)==len(ll)==len(mm)==len(nn)==len(oo)==60):
		data = np.array([aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll,mm,nn,oo])
		data = preprocessing.normalize(data)
		data = preprocessing.scale(data)
		pred=[]
		pred.append(data)
		pred=np.array(pred)
		model=generate_model()
		model.compile(loss='sparse_categorical_crossentropy', optimizer='adam', metrics=['accuracy'])
		model.load('my_model_weights.h5')
		prediction=model.predict(pred)
		temp=prediction.argmax()
		temp2=prediction.max()
		le = joblib.load(r'labenencoderweights.pkl')
		if(temp2>0.5):
		  value=le.inverse_transform([temp])[0]
		else :
		  value=-1
		return value
	else :
		return -2

@app.route('/api', methods=['GET','OPTIONS'])
def api():
    
    
    
    a = request.args.get("Long_Term_Fuel_Trim_Bank1")
    b=request.args.get("Intake_air_pressure")
    c = request.args.get("Accelerator_Pedal_value")
    d=request.args.get("Fuel_consumption")
    e = request.args.get("Torque_of_friction")
    f=request.args.get("Maximum_indicated_engine_torque")
    g = request.args.get("Engine_torque")
    h=request.args.get("Calculated_LOAD_value")
    i = request.args.get("Activation_of_Air_compressor")
    j=request.args.get("Engine_coolant_temperature")
    k = request.args.get("Transmission_oil_temperature")
    l=request.args.get("Wheel_velocity_front_left-hand")
    m = request.args.get("Wheel_velocity_front_right-hand")
    n=request.args.get("Wheel_velocity_rear_left-hand")
    o = request.args.get("Torque_converter_speed")

    aa =listo( aa , a )
	bb =listo( bb , b )
	cc =listo( cc , c )
	dd =listo( dd , d )
	ee =listo( ee , e )
	ff =listo( ff , f )
	gg =listo( gg , g )
	hh =listo( hh , h )
	ii =listo( ii , i )
	jj =listo( jj , j )
	kk =listo( kk , k )
	ll =listo( ll , l )
	mm =listo( mm , m )
	nn =listo( nn , n )
	oo =listo( oo , o )
    
    vv=do_predictions()

    print(str(a)," ",str(name))

    return str(vv)


if __name__ == '__main__':
    app.run(debug=True,host='0.0.0.0')
