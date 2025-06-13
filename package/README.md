PXF Packaging
============

PXF consists of 3 groups of artifacts, each developed using a different underlying technology:

* Greenplum extension -- written in C; when built, produces a `pxf.so` library and configuration files
* PXF Server -- written in Java; when built, produces a `pxf.war` file, Tomcat server, dependent JAR files, templates and scripts
* Script Cluster Plugin -- written in Go; when built, produces a `pxf-cli` executable

The PXF build system can create an RPM package on CentOs platform and a DEB package on Ubuntu platform,
respectively. PXF compiles against and generates a different package for every major Greenplum version.

For example, `pxf-gp6-1.2.3-1.el7.x86_64.rpm` represents an RPM package of PXF version 1.2.3 intended to work with
Greenplum 6 on Centos / Redhat 7 operating systems.

# How it works

We use docker as clean build environment. Build is consists of two stages:
1. Build open-gpdb / cloudberry and save deb file to `downloads/`
2. Build PXF using deb file from `downloads/` and save resulting deb to `downloads/`

## GP/PXF DEB specification

/gpdb_src/                 - github/open-gpdb/gpdb
/gpdb_src/debian           - one of the `debian` folders from github/open-gpdb/gpdb-devops
/gpdb_src/debian/build     - build destination for open-gpdb build

/pxf_src/                  - this repo
/pxf_src/devops            - git submodule github/open-gpdb/gpdb-devops
/pxf_src/debian/build/gp   - link to /opt/greenplum-db-<version>, pxf will install pxf.so here

`mk-build-deps` install build dependencies from debian/control file
`dpkg-buildpackage -us -uc` - build debian package (`-uc -us` - without signing)
basically it will call `make <step>` from debian/rules file
Resulting deb files consists of:
* files that `make install` from debian/rules copied to `$(DESTDIR)`
* files mentioned in debian/install


## Debugging

```
export BUILDX_EXPERIMENTAL=1
docker buildx debug --on=error build  -f package/pxf_6_jammy/Dockerfile .
...
BOOOM!

(buildx) list
(buildx) attach local
(buildx) exec /bin/bash
```

More info: https://github.com/docker/buildx/blob/master/docs/debugging.md


## PXF RPM specification
On Centos platforms PXF product is packaged as an RPM. The specification on how to build the RPM is provided by the
`pxf-gpX.spec` files in this directory. The following key design decisions were made:

* the name of the RPM package is `pxf-gpX`, where X is the major Greenplum version (e.g. `pxf-gp5`, `pxf-gp6`)
* to install a newer RPM package for the same Greenplum major release, a user will have to upgrade the PXF RPM
* the RPM installs PXF server into `/usr/local/pxf-gpX` directory (e.g. `/usr/local/pxf-gp6`)
* the RPM is relocatable, a user can specify --prefix option when installing the RPM to install the server into another directory
* the PXF greenplum extension is initially installed by RPM alongside the PXF server and is not initially active
* the PXF greenplum extension is copied into Greenplum install location during `pxf init` command issued by a user after the install
* the PXF RPM version number follows 3-number semantic versioning and must be provided during the RPM build process
* the PXF RPM release number is usually specified as `1`
* example PXF RPM names are : `pxf-gp5-1.2.3-1.el6.x86_64.rpm` and `pxf-gp5-1.2.3-1.el7.x86_64.rpm` 

## PXF RPM build process

To build an RPM, follow these steps:
1. Install the `rpm-build` package: `sudo yum install rpm-build`
2. Install Greenplum database
3. Run `source $GPHOME/greenplum_path.sh` to configure your `PATH` to be able to find `pg_config` program
4. Run `make clean rpm` from the top-level directory to build artifacts and assemble the RPM
5. The RPM will be available in `build/rpmbuild/RPMS` directory


## PXF RPM installation process
To install PXF from an RPM, follow these steps:
1. Build or download PXF RPM for the corresponding major version of Greenplum. The following example will assume
   that PXF version `1.2.3` will be installed to work with with Greenplum 5.
2. Decide which OS user will own the PXF installation. If PXF is installed alongside Greenplum, the user that owns the PXF
installation should either be the same as the one owning the Greenplum installation or have write privilleges to the
Greenplum installation directory. This is necessary to be able to register the PXF Greenplum extension with Greenplum.
3. If a previous PXF version has been installed, stop the PXF server.
4. As a superuser, run `rpm -Uvh pxf-gp5-1.2.3-1.el7.x86_64.rpm` to install the RPM into `/usr/local/pxf-gp5`
5. As a superuser, run `chown gpadmin:gpadmin /usr/local/pxf-gp5` to change ownership of PXF installation to the user `gpadmin`.
Specify a different user other than `gpadmin`, if desired.

After these steps, the PXF product will be installed and is ready to be configured. If there was a previous installation of
PXF for the same major Greenplum version, the files and the runtime directories from the older version will be removed.
The PXF configuration directory should remain intact. You will need to have Java installed to run the PXF server.

## PXF removal process
To remove the installed PXF package, follow these steps:
1. Stop the PXF server.
2. As a superuser, run `rpm -e pxf-gp5` (or `rpm -e pxf-gp6`). This will remove all files installed by the RPM package
and the PXF runtime directories. The PXF configuration directory should remain intact.
