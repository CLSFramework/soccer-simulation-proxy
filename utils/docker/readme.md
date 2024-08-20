# Create and Push Docker Image

To create an image based on Ubuntu 22.04, change the base image in the Dockerfile to `ubuntu:22.04` and then run the following commands (replace 20 to 22).
## Create Docker Image
    
    ```bash
    docker build -t <image-name>:<tag> .
    ```
    ```bash
    docker build -t naderzare/ubuntu20-grpc-thrift:latest
    ```

## Push Docker Image

    ```bash
    docker login
    docker push <image-name>:<tag>
    ```
    ```bash
    docker login
    docker push naderzare/ubuntu20-grpc-thrift:latest
    ```