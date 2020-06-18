# How to make a keyvi release (e.g., vX.Y.Z)

## Create a release branch
Create a release branch called `release-X.Y.Z`

## On the `release-X.Y.Z` branch

### Update the `python/setup.py` file
 - Update the flag `IS_RELEASED` to `True`
 - Commit to `release-X.Y.Z` and push it to https://github.com/KeyviDev/keyvi/
 - Wait for travis to build all targets

### Create tag 
 - Draft a new release tagged vX.Y.Z with `release-X.Y.Z` as the target branch
 - Add the release notes in the description with references to PRs
 - Publish release

## On the `master` branch

### Update the `python/setup.py` file
 - Update to the next release version 
```
VERSION_MAJOR = X
VERSION_MINOR = Y
VERSION_PATCH = Z + 1
VERSION_DEV = 0
```
### Update the `rust/Cargo.toml` file
 - Update to the same version used in `python/setup.py`
```
 version = "X.Y.Z + 1"
```
