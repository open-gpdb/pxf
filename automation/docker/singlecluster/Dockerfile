FROM ubuntu:18.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update &&  \
    apt-get install -y --no-install-recommends \
      curl ca-certificates

# TODO: update hive to support java 11+
ENV    HADOOP_VERSION=3.3.6
ENV      HIVE_VERSION=3.1.0
ENV ZOOKEEPER_VERSION=3.5.9
ENV     HBASE_VERSION=2.0.2
ENV       TEZ_VERSION=0.9.1

ENV APACHE_MIRROR="archive.apache.org"
ENV    HADOOP_URL="https://$APACHE_MIRROR/dist/hadoop/common/hadoop-$HADOOP_VERSION/hadoop-$HADOOP_VERSION.tar.gz"
ENV      HIVE_URL="https://$APACHE_MIRROR/dist/hive/hive-$HIVE_VERSION/apache-hive-$HIVE_VERSION-bin.tar.gz"
ENV ZOOKEEPER_URL="https://$APACHE_MIRROR/dist/zookeeper/zookeeper-$ZOOKEEPER_VERSION/apache-zookeeper-$ZOOKEEPER_VERSION-bin.tar.gz"
ENV     HBASE_URL="https://$APACHE_MIRROR/dist/hbase/$HBASE_VERSION/hbase-$HBASE_VERSION-bin.tar.gz"
ENV       TEZ_URL="https://$APACHE_MIRROR/dist/tez/$TEZ_VERSION/apache-tez-$TEZ_VERSION-bin.tar.gz"

ENV GPHD_ROOT=/home/gpadmin/workspace/singlecluster
ENV HADOOP_ROOT=$GPHD_ROOT/hadoop
ENV HBASE_ROOT=$GPHD_ROOT/hbase
ENV HIVE_ROOT=$GPHD_ROOT/hive
ENV ZOOKEEPER_ROOT=$GPHD_ROOT/zookeeper
ENV TEZ_ROOT=$GPHD_ROOT/tez

RUN mkdir -p $HADOOP_ROOT && \
    curl -fSL "$HADOOP_URL" -o hadoop.tar.gz && \
    tar xvf hadoop.tar.gz -C $HADOOP_ROOT --strip-components 1  --exclude="share/doc/*" --exclude="*-sources.jar" && \
    rm hadoop.tar.gz

RUN mkdir -p $HIVE_ROOT && \
    curl -fSL $HIVE_URL -o hive.tar.gz && \
    tar xvf hive.tar.gz -C $HIVE_ROOT --strip-components 1 && \
    rm hive.tar.gz

RUN mkdir -p $ZOOKEEPER_ROOT && \
    curl -fSL $ZOOKEEPER_URL -o zookeeper.tar.gz && \
    tar xvf zookeeper.tar.gz -C $ZOOKEEPER_ROOT --strip-components 1 --exclude="docs/*" && \
    rm zookeeper.tar.gz

RUN mkdir -p $HBASE_ROOT && \
    curl -fSL "$HBASE_URL" -o hbase.tar.gz && \
    tar xvf hbase.tar.gz -C $HBASE_ROOT --strip-components 1 --exclude="docs/*" && \
    rm hbase.tar.gz

RUN mkdir -p $TEZ_ROOT && \
    curl -fSL "$TEZ_URL" -o tez.tar.gz && \
    tar xvf tez.tar.gz -C $TEZ_ROOT --strip-components 1 && \
    rm tez.tar.gz

COPY docker/singlecluster/templates $GPHD_ROOT
COPY docker/singlecluster/conf $GPHD_ROOT/conf
COPY docker/singlecluster/bin $GPHD_ROOT/bin