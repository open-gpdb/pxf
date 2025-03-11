How it works

# We use docker as clean build environment.

/packaging/                - this repo
/packaging/debian          - github/open-gpdb/gpdb_deb/
/packaging/debian/build/gp - link to /opt/greenplum-db-<version>, pxf will install pxf.so here 

`mk-build-deps` install build dependencies from debian/control file
`dpkg-buildpackage -us -uc` - build debian package (`-uc -us` - without signing)
   basically it will call `make <step>` from debian/rules file
   Resulitng deb files conists of:
   * files that `make install` from debian/rules copied to `$(DESTDIR)`
   * files mentioned in debian/install


### debian folder structure

**/debian/source/format**   - always `3.0 (native)` - native debian package

**/debian/source/local-options**   - empty, we don't care about local changes made by dpkg, because it runs on copy inside docker

**/debian/compat**   - `9` - debhelper compatibility level. Same version should be in `debian/control` file.

Notes: upgrade to compat 10+?


## Debugging

```
docker buildx debug --on=error build  -f .github/package/pxf_6_jammy/Dockerfile .
...
BOOOM!

(buildx) list
(buildx) attach local
(buildx) exec /bin/bash
```

More info: https://github.com/docker/buildx/blob/master/docs/debugging.md