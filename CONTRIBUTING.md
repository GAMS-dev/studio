# Quality Gates

## Virtual Python Environment

Setup a virtual Python environment. Please check on your own, which one to use and how to setup the environment. Below `venv`is used on a Debian 12 machine.

```
sudo apt install python3-venv
```

Create a virtual environment.

```
python3 -m venv .venv
```

Ativate the virtual environment.

```
source .venv/bin/activate
```

## Install pre-commit

Please check the information about the pre-commit tool and follow the install steps given on the [project page](https://pre-commit.com/). Also check the `.pre-commit-config.yaml` to see which checks are currently used.

## Install detect-secrets

Please check the information about the detect-secrets tool and follow the install steps given on the [project page](hhttps://github.com/Yelp/detect-secrets).
