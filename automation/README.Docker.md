# Running Automation in Docker

## How to run Automation in Docker:

```bash
cd automation
make copy-debs
docker compose build singlecluster
docker compose build universe
docker compose up
```

Investigate any issues:
`docker compose ps` + `docker exec -it <id> bash`

Extract logs:
```
mkdir -p  ./test_artifacts
docker compose cp universe:/home/gpadmin/workspace/pxf/automation/target/surefire-reports ./test_artifacts || echo "No surefire-reports found"
```

Hadoop logs in `/home/gpadmin/workspace/singlecluster/storage/logs/`

## How it works

The `docker-compose.yml` file defines the services needed to run the Automation tests. It includes:
- `universe` - docker container with ALL components: GP, PXF, singleCluster (HDFS, Hive, HBASE, etc.)

### Whole universe docker container

File layout:
```
/home/gpadmin
/home/gpadmin/pxf       - is PXF_BASE
/home/gpadmin/pxf/conf
/home/gpadmin/workspace/singlecluster - is GPHD_ROOT

/opt/greenplum-pxf-6/   - is PXF_HOME
```