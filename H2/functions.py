import torch
from torch import Tensor


def rastrigin(x: Tensor) -> Tensor:
    return 10 * x.shape[1] + (torch.pow(x,2) - 10 * torch.cos(2 * torch.pi * x)).sum(dim=1)


def griewangk(x: Tensor) -> Tensor:
    sqrt_i = (Tensor(list(range(x.shape[1]))).expand(x.shape[0],-1) + 1).sqrt_().to(device=x.device)
    return (torch.pow(x,2) / 4000).sum(dim=1) - (torch.cos(x / sqrt_i)).prod(dim=1) + 1


def michalewicz(x: Tensor) -> Tensor:
    m = 2
    i_vec = (Tensor(list(range(x.shape[1]))).expand(x.shape[0],-1) + 1).to(device=x.device) / torch.pi
    return -1 * (torch.sin(x) * torch.pow(torch.sin(x * i_vec),2*m)).sum(dim=1)


def rosenbrock(x: Tensor) -> Tensor:
    x_i1 = x[:,1:]
    x_i = x[:,:-1]
    return (100 * torch.pow(x_i1 - torch.pow(x_i, 2), 2) + torch.pow(1 - x_i, 2)).sum(dim=1)


funclib = {
    'rastrigin':   (rastrigin,-5.12, 5.12),
    'griewangk':   (griewangk,-600, 600),
    'michalewicz': (michalewicz,0,torch.pi),
    'rosenbrock':  (rosenbrock, -2.048, 2.048),
}


if __name__ == '__main__':
    x = Tensor([[0,0,0],[0.2,0.1,0],[1,6,2],[1,1,1]]).to(torch.device('cuda'))
    print(x.shape)
    print(griewangk(x, torch.device('cuda')).shape)
