#!/bin/bash
function usage() {
  cat <<-EOF
Usage: $0 -i <docker image>
  Enters the docker container at the current directory
EOF
}

while getopts "i:" opt; do
    case $opt in
        i) DOCKER_IMAGE=${OPTARG} ;;
      *|?) usage && exit 1;;
    esac
done

if [ -z "$DOCKER_IMAGE" ]
then
  usage && exit 1
fi

docker run -v $(pwd):$(pwd) -w $(pwd) -it ${DOCKER_IMAGE} /bin/bash
