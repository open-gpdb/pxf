# Running Automation in Docker

## How to run Automation in Docker:

```bash
cd automation
make copy-debs
docker compose build
docker compose up
```

## How it works

The `docker-compose.yml` file defines the services needed to run the Automation tests. It includes:
- `universe` - docker container with ALL components: GP, PXF, singleCluster (HDFS, Hive, HBASE, etc.)

### Whole universe docker container

File layout:
```
/home/gpadmin
/home/gpadmin/pxf       - is PXF_BASE
/home/gpadmin/pxf/conf

/opt/greenplum-pxf-6/   - is PXF_HOME

/home/gpadmin/workspace/singlecluster - ???
```