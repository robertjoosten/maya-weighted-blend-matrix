# maya-weighted-blend-matrix
Maya weighted blend matrix node that will allow blending of the transformation
components separately. Rather than an additive approach like Maya's build in
`blendMatrix` node its behaviour is more similar to a normal constraint where 
the `translate`, `scale` and `shear` components are blended using linear 
interpolation and the `rotation` is solved using average quaternions. Each 
component can be blended using separated weighting making this node a good 
candidate for space switch as it doesn't require separate constraints for
different components.

## Installation
* Extract the content of the .rar file anywhere on disk.
* Build the plugin for a specific Maya version.
* Drag the weighted-blend-matrix.mel file in Maya to permanently install the script.

## Compiling
Building the plugin using cmake will place the plugin in the plug-ins folder 
with a maya version divider. This will ensure the plug-in is compatible with 
the launched version of Maya. 

1. Copy the following libraries to their respective folders in `/extern`:
    - [Eigen 3.3.9](https://eigen.tuxfamily.org/) with path `/extern/Eigen/Eigen/Dense`,
 
2. Open Terminal
```
cd <PATH_TO_MODULE>
mkdir build/<MAYA_VERSION>
cd build/<MAYA_VERSION>
cmake -A x64 -T v141 -DMAYA_VERSION=<MAYA_VERSION> ../../
cmake --build . --target install --config Release
```

## Usage
Once the plug-in is build and loaded a new `wtBlendMatrix` node can be 
created. There is a compound array attribute that contains `blendInputMatrix`, 
`blendOffsetMatrix`, `blendTranslateWeight`, `blendRotateWeight`, 
`blendScaleWeight` and `blendShearWeight` attributes. By default the weight 
values are set to 1. The `blendInputMatrix` can be driven directly by a nodes 
world matrix attribute and `blendOffsetMatrix` can be calculated and set by 
the user to get the matrix in the desired location. 

<p align="center"><img src="icons/weighted-blend-matrix-compound-example.png?raw=true"></p>

At the top level the node has a `restMatrix` which is used when the sum of 
weights on a component is 0. The `offsetMatrix` can be used as an offset 
matrix that is used in the calculation after the blend matrix itself has been
calculated, as a matrix attribute is not keyable a `composeMatrix` node can 
be used to preserve the output location if using the node as a space switch.

<p align="center"><img src="icons/weighted-blend-matrix-network-example.png?raw=true"></p>

When using Maya 2020 or above the `decomposeMatrix` and `composeMatrix` nodes 
can be omitted as the `outputMatrix` attribute can be plugged directly into 
the `offsetParentMatrix` which leaves the translate/rotate/scale and shear 
attributes free to key the offset.

