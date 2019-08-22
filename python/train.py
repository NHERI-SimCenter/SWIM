# -*- coding: utf-8 -*-
"""
/*------------------------------------------------------*
| This script is used to train a neural net to predict  |
| SAM parameters given BIM features.                    |
|                                                       |
| Author: Charles Wang,  UC Berkeley c_w@berkeley.edu   |
|                                                       |
| Date:   08/20/2019                                    |
*------------------------------------------------------*/
"""

from __future__ import absolute_import, division, print_function
import pathlib
import pandas as pd
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt

from nnmodel import *
import random




# define paths
dataset_path = '../data/DatasetV1_ContinuumWall.txt'
label_path = '../data/Labels_ContinuumWall.txt'



# load data
column_names = ['height', 'length', 'thickness', 'E', 'fc'] 
raw_dataset = pd.read_csv(dataset_path, names=column_names,
                      na_values = "?", comment='\t',
                      sep=",", skipinitialspace=True)
dataset = raw_dataset.copy()
print(dataset.tail())

# load labels:   column_names= ['Name', Ap', 'An', 'Bn', 'beta', 'N']
raw_labelset = pd.read_csv(label_path, 
                      na_values = "?", comment='\t',
                      sep=",", skipinitialspace=True)
labelset = raw_labelset[['Ap', 'An', 'Bn', 'beta','N']].copy()
print(labelset.tail())


# merge label into dataset
dataset = pd.concat([dataset, labelset], axis=1, join='inner')
print(dataset.tail())

'''
# pertubate the data
nr = 100 # num of randomly generated data
columList = ['height','length','thickness','E','fc','Ap','An','Bn','beta','N']
for index, row in dataset.iterrows():
    for i in range(nr):
        vList = []
        for v in columList:
            vList.append(np.random.normal(row[v],row[v]*0.01,1)[0])
        vList[-1] = row['N']
        df2 = pd.DataFrame([vList], columns=columList)
        dataset = dataset.append(df2, ignore_index=True)
'''


# Split data into train and test
train_dataset = dataset.sample(frac=0.8,random_state=100)
test_dataset = dataset.drop(train_dataset.index)

'''
# Inspect data
# joint distributions
sns.pairplot(train_dataset[['height', 'length', 'thickness', 'E', 'fc']], diag_kind="kde")
'''

# statistics
train_stats = train_dataset.describe()
train_stats.pop("Ap")
train_stats.pop("An")
train_stats.pop("Bn")
train_stats.pop("beta")
train_stats.pop("N")
train_stats = train_stats.transpose()
print(train_stats)
store = pd.HDFStore('../data/DataStats_ContinuumWall_V1.h5')
store['stats'] =  train_stats


# Split features from labels
train_labels = train_dataset[['Ap','An', 'Bn', 'beta','N']].copy()
train_dataset = train_dataset.drop(['Ap','An', 'Bn', 'beta','N'], axis=1)
test_labels = test_dataset[['Ap','An', 'Bn', 'beta','N']].copy()
test_dataset = test_dataset.drop(['Ap','An', 'Bn', 'beta','N'], axis=1)



# Normalize data
def norm(x):
  return (x - train_stats['mean']) / train_stats['std']
normed_train_data = norm(train_dataset)
normed_test_data = norm(test_dataset)


dataset = []
for data in normed_train_data.to_numpy():
    dataset.append(torch.from_numpy(data))

labelset = []
for data in train_labels.to_numpy():
    labelset.append(torch.from_numpy(data))





train = True
test = True

inputSize = 5
outputSize = 5
modelFile_pth = '../cpp/MyResources/model.pth'
modelFile_pt = "../cpp/MyResources/model.pt"
if train:
    # Training
    num_epochs = 100
    learning_rate = 0.01
    # Build the model
    model = nnmodel(inputSize=inputSize, outputSize=outputSize)#.cuda()

    criterion = nn.MSELoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=learning_rate)
    #optimizer = torch.optim.SGD(model.parameters(), lr = learning_rate)
    #mymonitor = monitor()
    err = []
    for epoch in range(num_epochs):
        for i in range(len(dataset)):
            data = dataset[i]
            data = data.view(1, -1)
            data = Variable(data)#.cuda()
            label = labelset[i]
            label = label.view(1, -1)
            label = Variable(label)#.cuda()
            # ===================forward=====================
            output = model(data.float())
            loss = criterion(output, label.float())
            # ===================backward====================
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
        # ===================log========================
        if epoch % 10 == 0: 
            print('epoch [{}/{}], loss:{:.4f}'.format(epoch + 1, num_epochs, loss.item()))
        err.append(loss.item())
        """
        if epoch % 10 == 0:
            np.savetxt('../data/tmp/'+ str(epoch+1) +'.txt',output.cpu().detach().numpy())
            '''
            pic = to_img(output.cpu().data)
            save_image(pic, './mlp_img/image_{}.png'.format(epoch))
            '''
        """
        #mymonitor.update(loss.item())

    plt.figure()
    plt.plot(err)
    plt.show()
    plt.pause(0.001)
    #input("Press enter to continue...")

    torch.save(model.state_dict(), modelFile_pth)

    
    # An example input you would normally provide to your model's forward() method.
    example = torch.rand(1, inputSize)
    # Use torch.jit.trace to generate a torch.jit.ScriptModule via tracing.
    traced_script_module = torch.jit.trace(model, example)
    traced_script_module.save(modelFile_pt)

if test: 

    # testing

    model = nnmodel(inputSize=inputSize, outputSize=outputSize)#.cuda()
    model.load_state_dict(torch.load(modelFile_pth))
    predictedY = []
    for data in dataset:
            data = data.view(1, -1)
            data = Variable(data)#.cuda()
            # ===================forward=====================
            y = model.forward(data.float()).detach().numpy()[0]
            predictedY.append(y)
    predictedY = np.asarray(predictedY)
    print('xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx')


# forward
traced_script_module = torch.jit.load(modelFile_pt)
x = torch.tensor([[0.9321, 0.1068, 0.2950, 1.0220, 1.0184], [0.9321, 0.1068, 0.2950, 1.0220, 1.0184]])
output = traced_script_module(x)
print(output)



print(dataset[0])
print(labelset[0])
print(predictedY[0])

exit()


# Predict
test_predictions = predictedY.flatten()
test_labels = train_labels

plt.figure(figsize=(20,10))
plt.subplot(2,5,1)
trueValues = test_labels['Ap']
predictValues = test_predictions[0::5]
print(trueValues, predictValues)
plt.scatter(trueValues, predictValues)
plt.xlabel('True Values [Ap]')
plt.ylabel('Predictions [Ap]')
plt.axis('equal')
plt.axis('square')
plt.xlim([0,plt.xlim()[1]])
plt.ylim([0,plt.ylim()[1]])
_ = plt.plot([-100, 100], [-100, 100])

plt.subplot(2,5,6)
error = predictValues - trueValues
plt.hist(error, bins = 25)
plt.xlabel("Prediction Error [Ap]")
_ = plt.ylabel("Count")

plt.subplot(2,5,2)
trueValues = test_labels['An']
predictValues = test_predictions[1::5]
plt.scatter(trueValues, predictValues)
plt.xlabel('True Values [An]')
plt.ylabel('Predictions [An]')
plt.axis('equal')
plt.axis('square')
plt.xlim([0,plt.xlim()[1]])
plt.ylim([0,plt.ylim()[1]])
_ = plt.plot([-100, 100], [-100, 100])

plt.subplot(2,5,7)
error = predictValues - trueValues
plt.hist(error, bins = 25)
plt.xlabel("Prediction Error [An]")
_ = plt.ylabel("Count")

plt.subplot(2,5,3)
trueValues = test_labels['Bn']
predictValues = test_predictions[2::5]
plt.scatter(trueValues, predictValues)
plt.xlabel('True Values [Bn]')
plt.ylabel('Predictions [Bn]')
plt.axis('equal')
plt.axis('square')
plt.xlim([0,plt.xlim()[1]])
plt.ylim([0,plt.ylim()[1]])
_ = plt.plot([-100, 100], [-100, 100])

plt.subplot(2,5,8)
error = predictValues - trueValues
plt.hist(error, bins = 25)
plt.xlabel("Prediction Error [Bn]")
_ = plt.ylabel("Count")


plt.subplot(2,5,4)
trueValues = test_labels['beta']
predictValues = test_predictions[3::5]
plt.scatter(trueValues, predictValues)
plt.xlabel('True Values [beta]')
plt.ylabel('Predictions [beta]')
plt.axis('equal')
plt.axis('square')
plt.xlim([0,plt.xlim()[1]])
plt.ylim([0,plt.ylim()[1]])
_ = plt.plot([-100, 100], [-100, 100])

plt.subplot(2,5,9)
error = predictValues - trueValues
plt.hist(error, bins = 25)
plt.xlabel("Prediction Error [beta]")
_ = plt.ylabel("Count")


plt.subplot(2,5,5)
trueValues = test_labels['N']
predictValues = test_predictions[4::5]
plt.scatter(trueValues, predictValues)
plt.xlabel('True Values [N]')
plt.ylabel('Predictions [N]')
plt.axis('equal')
plt.axis('square')
plt.xlim([0,plt.xlim()[1]])
plt.ylim([0,plt.ylim()[1]])
_ = plt.plot([-100, 100], [-100, 100])

plt.subplot(2,5,10)
error = predictValues - trueValues
plt.hist(error, bins = 25)
plt.xlabel("Prediction Error [N]")
_ = plt.ylabel("Count")

plt.savefig('../figures/NN_ContinuumWall_Predictions_V1.png')
plt.show()
#input("Press enter to continue...")
plt.close()

