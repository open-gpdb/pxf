package org.greenplum.pxf.plugins.iceberg.converters;

import org.apache.iceberg.types.Type;
import org.greenplum.pxf.api.io.DataType;

public interface IcebergConverter<V> {

    Type getIcebergType();

    DataType getCloudberryType();

    V convertFromCloudberryToIceberg(Object input);

    Object convertFromIcebergToCloudberry(Object input);
}
