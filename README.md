# LifeBrush
LifeBrush is a VR Unreal Engine 4 toolkit for creating interactive biomolecular illustrative simulations. It combines interactive painting of molecular patterns in VR, with an agent-based modeling framework for defining molecular interactions and behaviours. 

LifeBrush is fast. The generative brushes can synthesize thousands of molecules per second in volumes and on surfaces. The agent-based modeling framework is also fast, capable of simulating more than 10,000 agents at 90 fps in VR.

A user interacts with LifeBrush through a VR interface and the Unreal Engine Editor interface. In the VR interface, you paint agents in 3D space from an example-palette (it's like Tilt Brush, but for agents). In the Unreal Engine Editor interface, one configures the agents.

The C++ agent framework is based on an Entity-Component-System that we developed for Unreal Engine. Components are compact C++ structs. Entities organize multiple components into a single entity. We do not use Unreal's actor-component model because it cannot handle very large numbers of agents. Our ECS is the basis of our brush-based synthesis framework for painting agents and for simulating them.

This toolkit was orignally described in a conference paper in 2018, "LifeBrush: Painting interactive agent-based simulations" by Timothy Davison, Faramarz Samavati and Christian Jacob (the bibtex is below). The version here on GitHub is an evolution of that paper.

# License and Copyright 

All code, unless stated otherwise, is Copyright (c) 2019, Timothy Davison. All rights reserved.

All of my code is released under a MIT license. There are included source-codes released under their respective licenses.

**Please cite with:**
```
@inproceedings{davison2018lifebrush,
  title={LifeBrush: Painting interactive agent-based simulations},
  author={Davison, Timothy and Samavati, Faramarz and Jacob, Christian},
  booktitle={2018 International Conference on Cyberworlds (CW)},
  pages={17--24},
  year={2018},
  organization={IEEE}
}
```
