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

## Secret detection with betterleaks

The pre-commit hook uses [betterleaks](https://github.com/betterleaks/betterleaks) to scan staged changes for secrets. pre-commit builds the tool automatically, a separate installation isn't needed.

If betterleaks reports a false positive, either add a `betterleaks:allow` comment to the line or add the fingerprint of the finding to the `.betterleaksignore` file in the repository root.
