#include "diskmanager.h"

#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "common/filefunctions.h"
#include "config/config.h"

DiskManager* DiskManager::instance_ = nullptr;

DiskManager::DiskManager() :
  consumption_(0)
{
  // Try to load any current cache index from file
  QFile cache_index_file(QDir(GetMediaCacheLocation()).filePath("index"));

  if (cache_index_file.open(QFile::ReadOnly)) {
    QDataStream ds(&cache_index_file);

    while (!cache_index_file.atEnd()) {
      HashTime h;

      ds >> h.file_name;
      ds >> h.hash;
      ds >> h.access_time;
      ds >> h.file_size;

      if (QFileInfo::exists(h.file_name)) {
        consumption_ += h.file_size;
        disk_data_.append(h);
      }
    }
  }
}

DiskManager::~DiskManager()
{
  if (Config::Current()["ClearDiskCacheOnClose"].toBool()) {
    // Clear all cache data
    ClearDiskCache();
  } else {
    // Save current cache index
    QFile cache_index_file(QDir(GetMediaCacheLocation()).filePath("index"));

    if (cache_index_file.open(QFile::WriteOnly)) {
      QDataStream ds(&cache_index_file);

      foreach (const HashTime& h, disk_data_) {
        ds << h.file_name;
        ds << h.hash;
        ds << h.access_time;
        ds << h.file_size;
      }
    } else {
      qWarning() << "Failed to write cache index";
    }
  }
}

void DiskManager::CreateInstance()
{
  instance_ = new DiskManager();
}

void DiskManager::DestroyInstance()
{
  delete instance_;
  instance_ = nullptr;
}

DiskManager *DiskManager::instance()
{
  return instance_;
}

void DiskManager::Accessed(const QByteArray &hash)
{
  for (int i=disk_data_.size()-1;i>=0;i--) {
    const HashTime& h = disk_data_.at(i);

    if (h.hash == hash) {
      HashTime moved_hash = h;

      moved_hash.access_time = QDateTime::currentMSecsSinceEpoch();

      disk_data_.removeAt(i);
      disk_data_.append(moved_hash);
      break;
    }
  }
}

void DiskManager::Accessed(const QString &filename)
{
  for (int i=disk_data_.size()-1;i>=0;i--) {
    const HashTime& h = disk_data_.at(i);

    if (h.file_name == filename) {
      HashTime moved_hash = h;

      moved_hash.access_time = QDateTime::currentMSecsSinceEpoch();

      disk_data_.removeAt(i);
      disk_data_.append(moved_hash);
      break;
    }
  }
}

void DiskManager::CreatedFile(const QString &file_name, const QByteArray &hash)
{
  qint64 file_size = QFile(file_name).size();

  disk_data_.append({file_name, hash, QDateTime::currentMSecsSinceEpoch(), file_size});

  consumption_ += file_size;

  while (consumption_ > DiskLimit()) {
    DeleteLeastRecent();
  }
}

bool DiskManager::ClearDiskCache()
{
  disk_data_.clear();

  return QDir(GetMediaCacheLocation()).removeRecursively();
}

void DiskManager::DeleteLeastRecent()
{
  HashTime h = disk_data_.takeFirst();

  QFile::remove(h.file_name);

  consumption_ -= h.file_size;

  emit DeletedFrame(h.hash);
}

qint64 DiskManager::DiskLimit()
{
  double gigabytes = Config::Current()["DiskCacheSize"].toDouble();

  // Convert gigabytes to bytes
  return qRound64(gigabytes * 1073741824);
}