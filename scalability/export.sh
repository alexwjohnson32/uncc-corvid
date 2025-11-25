#!/bin/bash

export_dir=$1
export_gld=$export_dir/IEEE_8500
export_gpk=$export_dir/IEEE_118

echo "Exporting template files to $export_dir."

mkdir -p $export_dir $export_gld $export_gpk

cp build/bin/gridlabd/baseline_IEEE_8500.glm $export_gld/
cp build/bin/gridlabd/json/IEEE_8500node.json $export_gld/

cp build/bin/gridpack/IEEE-118/118.raw $export_gpk/
cp build/bin/gridpack/IEEE-118/118.xml $export_gpk/
cp build/bin/gridpack/IEEE-118/helics_setup.json $export_gpk/
# cp build/bin/gridpack/IEEE-118/helics_fed_info_config.json $export_gpk/
cp build/bin/gridpack/IEEE-118/powerflow_ex.x $export_gpk/