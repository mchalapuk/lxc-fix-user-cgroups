# lxc-fix-user-cgroups
workaround for cgroups problems when using unprivileged lxc containers with new new systemd

(see https://bugs.launchpad.net/ubuntu/+source/systemd/+bug/1346734)

## Usage

Simply run this program in shell before starting or creating a container (once for a shell is enough).
