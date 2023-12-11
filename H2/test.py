import numpy as np
import matplotlib.pyplot as plt
import torch

# def spread_adjust(z):
#     factor = 3
#     z = z * 2 * factor - factor  # get the numbers from [0,1) -> [-factor,factor)
#     return 1/(1 + np.exp(-z))  # apply sigmoid to the new numbers

def spread_adjust(z: torch.Tensor, factor: float = 3):
    z = z.mul_(2 * factor).sub_(factor)  # get the numbers from [0,1) -> [-factor,factor)
    return z.sigmoid_()  # apply sigmoid to the new numbers - this changes the distribution of the numbers


a = torch.rand((50000000,2))
spread_adjust(a,0.9)
plt.hist2d(a[:,0],a[:,1],density=True,bins=(50,50))
plt.show()