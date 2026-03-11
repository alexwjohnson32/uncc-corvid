
#--build
SIF_FILE=$1
/opt/Software/bin/apptainer exec $SIF_FILE bash -s <<'EOF'
gridlabd.sh 1a.glm
EOF