#include "ElevationMapProvider.h"
#if defined(DEBUG_GOOGLE_MAPS)
#include <QFile>
#include <QStandardPaths>
#endif
#include "QGCMapEngine.h"
#include "TerrainTile.h"

ElevationProvider::ElevationProvider(const QString& imageFormat, quint32 averageSize, QGeoMapType::MapStyle mapType, const QString &referrer, QObject* parent)
    : MapProvider(referrer, imageFormat, averageSize, mapType, parent) {}

//-----------------------------------------------------------------------------
int AirmapElevationProvider::long2tileX(const double lon, const int z) const {
    Q_UNUSED(z)
    return static_cast<int>(floor((lon + 180.0) / TerrainTile::tileSizeDegrees));
}

//-----------------------------------------------------------------------------
int AirmapElevationProvider::lat2tileY(const double lat, const int z) const {
    Q_UNUSED(z)
    return static_cast<int>(floor((lat + 90.0) / TerrainTile::tileSizeDegrees));
}

QString AirmapElevationProvider::_getURL(const int x, const int y, const int zoom, QNetworkAccessManager* networkManager) {
    Q_UNUSED(networkManager)
    Q_UNUSED(zoom)
    return QString("https://api.airmap.com/elevation/v1/ele/carpet?points=%1,%2,%3,%4")
        .arg(static_cast<double>(y) * TerrainTile::tileSizeDegrees - 90.0)
        .arg(static_cast<double>(x) * TerrainTile::tileSizeDegrees - 180.0)
        .arg(static_cast<double>(y + 1) * TerrainTile::tileSizeDegrees - 90.0)
        .arg(static_cast<double>(x + 1) * TerrainTile::tileSizeDegrees - 180.0);
}

QGCTileSet AirmapElevationProvider::getTileCount(const int zoom, const double topleftLon,
                                                 const double topleftLat, const double bottomRightLon,
                                                 const double bottomRightLat) const {
    QGCTileSet set;
    set.tileX0 = long2tileX(topleftLon, zoom);
    set.tileY0 = lat2tileY(bottomRightLat, zoom);
    set.tileX1 = long2tileX(bottomRightLon, zoom);
    set.tileY1 = lat2tileY(topleftLat, zoom);

    set.tileCount = (static_cast<quint64>(set.tileX1) -
                     static_cast<quint64>(set.tileX0) + 1) *
                    (static_cast<quint64>(set.tileY1) -
                     static_cast<quint64>(set.tileY0) + 1);

    set.tileSize = getAverageSize() * set.tileCount;

    return set;
}

QByteArray AirmapElevationProvider::serializeTile(QByteArray image) {
    return TerrainTile::serializeFromAirMapJson(image);
}

int ApStr1ElevationProvider::long2tileX(const double lon, const int z) const {
    Q_UNUSED(z)
    qDebug() << "long2tileX, input lon: " << lon << " output tileX: " << static_cast<int>(floor((lon + 180.0)));
    return static_cast<int>(floor((lon + 180.0)));
}

//-----------------------------------------------------------------------------
int ApStr1ElevationProvider::lat2tileY(const double lat, const int z) const {
    Q_UNUSED(z)
    qDebug() << "lat2tileY, input lat: " << lat << " output tileY: " << static_cast<int>(floor((lat + 90.0)));
    return static_cast<int>(floor((lat + 90.0)));
}

//-----------------------------------------------------------------------------
QString ApStr1ElevationProvider::_getURL(const int x, const int y, const int zoom, QNetworkAccessManager* networkManager) {
    Q_UNUSED(networkManager)
    Q_UNUSED(zoom)

    QString formattedString1;
    QString formattedString2;

    // For saving them internally we do 0-360 and 0-180 to avoid signs. Need to redo that to obtain proper format for call
    int xForUrl = x - 180;
    int yForUrl = y - 90;

    formattedString1 = ( yForUrl > 0 ) ? QString("N%1").arg(QString::number(yForUrl).rightJustified(2, '0')) :
                                 QString("S%1").arg(QString::number(-yForUrl).rightJustified(2, '0'));

    formattedString2 = ( xForUrl > 0 ) ? QString("E%1").arg(QString::number(xForUrl).rightJustified(3, '0')) :
                                 QString("W%1").arg(QString::number(-xForUrl).rightJustified(3, '0'));

    QString urlString = QString("https://terrain.ardupilot.org/SRTM1/%1%2.hgt.zip")
         .arg(QString(formattedString1))
         .arg(QString(formattedString2))
         ;

    return urlString;
}

//--------------------------------------------------------------------------------
QGCTileSet ApStr1ElevationProvider::getTileCount(const int zoom, const double topleftLon,
                                                 const double topleftLat, const double bottomRightLon,
                                                 const double bottomRightLat) const {
    QGCTileSet set;
    set.tileX0 = long2tileX(topleftLon, zoom);
    set.tileY0 = lat2tileY(bottomRightLat, zoom);
    set.tileX1 = long2tileX(bottomRightLon, zoom);
    set.tileY1 = lat2tileY(topleftLat, zoom);

    set.tileCount = (static_cast<quint64>(set.tileX1) -
                     static_cast<quint64>(set.tileX0) + 1) *
                    (static_cast<quint64>(set.tileY1) -
                     static_cast<quint64>(set.tileY0) + 1);

    set.tileSize = getAverageSize() * set.tileCount;

    return set;
}


