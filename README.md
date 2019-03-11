# A Driver Identification FrameWork On AutoMotive Grade Linux

----
## what is WhoKnows?


> WhoKnows is a driver identification and classification framework build on Automotive Grade Linux that can be ported on Raspberry pi 3, the identification model is trained using neural networks (CNN/RNN) .

----
## usage
1. Collect data on drivers.
2. Train the driver identification/classification model using the provided architecture.
3. Setup the personalised drivers profiles on AGL.
4. Use the provided Flask server to communicate between Automotive Grade Linux and the driver identification model.

#Training the Driver Identification model

If you want to retrain the model using the FCN-LSTM architecture you can use the model in the training folder, it contains:

1. The driver identification model using FCN-LSTM.
2. The implemented anomalies detection using One-Class SVM.
3. The tests of the performance of the model with some anomalies injected in the data.

The used data in that example is the security dataset, it's available for download from this link:
http://ocslab.hksecurity.net/Datasets/driving-dataset

----
## Requirements

* Automotive Grade Linux
* Python 3

