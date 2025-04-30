# roi-uncc
Repository for students and researchers working on the ROI project at UNCC.



# Docker Image with Project Toolsets

The various software tools required for this project are included in a docker image built by the [Dockerfile](docker/Dockerfile) given in roi-uncc/docker. This image is built by executing:

```bash
docker build -t roi-img .
```
in the directory Dockerfile exists in.

The image can be run using :

```bash
docker run --rm -v /ABSOLUTE/PATH/ON/HOST:/ABSOLUTE/PATH/ON/CONTAINER -i -t roi-img /bin/bash
```
This binds a local host directory to a directory in the container and is a good way to persist files between different invocations of this image.

The included open source softwares are: HELICS, GridLab-D and GridPACK.


# Pushing Files to Repository.

Collaboraters are encouraged to push commits into this repository.

The workflow is to push your own local developement branches to GitHub, never push into main. It is recommened to rebase before pushing for clarity.
First you must authenticate with GitHub through SSH.

## GitHub  SSH Authentication in Linux

The following steps were tested in Ubuntu 24.04.1 LTS.

1. Generate private and public keys if you don't already have them in your ~/.ssh directory: 

```bash
ssh-keygen -t ed25519 -C "your-email@example.com"
```

2. Extract the contents of `~/.ssh/id_ed25519.pub` and add to `settings->SSH and GPG keys` at github.com.

3. Add the private key to your ssh agent:

```bash
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_ed25519
```

4. Confirm that GitHub recognizes your key with `ssh -T git@github.com`

5. In your local clone if the remote does not link to the ssh url (can be checked with `git remote -v`), set it with:

```bash
git remote set-url origin git@github.com:YOUR-USERNAME/roi-uncc.git
```
## Creating SIF File for RL8
Navigate to the root of the uncc repo

Run ```apptainer build --fakeroot --tmpdir ~/apptainerTmp/ rl8_uncc.sif rl8_uncc.def```









