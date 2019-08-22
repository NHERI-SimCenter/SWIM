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

import torch
import torchvision
from torch import nn
from torch.autograd import Variable
from torch.utils.data import DataLoader
from torchvision import transforms
from torchvision.datasets import MNIST
from torchvision.utils import save_image

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.path import Path
import matplotlib.patches as patches


class nnmodel(nn.Module):

    def __init__(self, inputSize, outputSize):
        super(nnmodel, self).__init__()
        '''
        if inputSize<128:
            print("inputsize can not be less than 128. quit.")
            exit()
        '''
        self.model = nn.Sequential(
            nn.Linear(inputSize, 5),
            nn.ReLU(True),
            nn.Linear(5, 64),
            nn.ReLU(True),
            nn.Linear(64, 64),
            nn.ReLU(True),
            nn.Linear(64, 64),
            nn.ReLU(True),
            nn.Linear(64, outputSize))#nn.Tanh())
            # add a layer nn.Softplus(True) at end will be fine

    def forward(self, x):
        x = self.model(x)
        '''
        pt = x.detach().numpy()[0]
        plt.plot([0],pt,'r.')
        #ax.scatter(pt[0],pt[1],pt[2], c='r', marker='o')
        plt.draw()
        plt.pause(0.0005)
        #print(pt)
        '''
        return x
    
    def predict(self, x):
        x = self.model(x)
        return x

def my_plotter(ax, data1, data2, param_dict):
    """
    A helper function to make a graph

    Parameters
    ----------
    ax : Axes
        The axes to draw to

    data1 : array
       The x data

    data2 : array
       The y data

    param_dict : dict
       Dictionary of kwargs to pass to ax.plot

    Returns
    -------
    out : list
        list of artists added
    """
    out = ax.plot(data1, data2, **param_dict)
    return out

class monitor():

    def __init__(self):
        self.lossList = []
        plt.plot([0,1],[0,1])
        plt.show()
        
        

    def update(self, loss):
        plt.clf() # clean figure
        self.lossList.append(loss)
        plt.plot(self.lossList, c='r')
        plt.draw()