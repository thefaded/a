#include "playlistmodel.h"

#include <QFileInfo>
#include <QUrl>
#include <QMediaPlaylist>
#include <QStringListModel>
#include <QMimeData>
#include <QDataStream>

PlaylistModel::PlaylistModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

PlaylistModel::~PlaylistModel()
{
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    return m_playlist && !parent.isValid() ? m_playlist->mediaCount() : 0;
}

int PlaylistModel::columnCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? ColumnCount : 0;
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    return m_playlist && !parent.isValid()
            && row >= 0 && row < m_playlist->mediaCount()
            && column >= 0 && column < ColumnCount
        ? createIndex(row, column)
        : QModelIndex();
}

QModelIndex PlaylistModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);

    return QModelIndex();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && role == Qt::DisplayRole) {
        QVariant value = m_data[index];
        if (!value.isValid() && index.column() == Title) {
            QUrl location = m_playlist->media(index.row()).canonicalUrl();
            return QFileInfo(location.path()).fileName();
        }

        return value;
    }
    return QVariant();
}

QMediaPlaylist *PlaylistModel::playlist() const
{
    return m_playlist.data();
}

void PlaylistModel::setPlaylist(QMediaPlaylist *playlist)
{
    if (m_playlist) {
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeInserted, this, &PlaylistModel::beginInsertItems);
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaInserted, this, &PlaylistModel::endInsertItems);
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeRemoved, this, &PlaylistModel::beginRemoveItems);
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaRemoved, this, &PlaylistModel::endRemoveItems);
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaChanged, this, &PlaylistModel::changeItems);
    }

    beginResetModel();
    m_playlist.reset(playlist);

    if (m_playlist) {
        connect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeInserted, this, &PlaylistModel::beginInsertItems);
        connect(m_playlist.data(), &QMediaPlaylist::mediaInserted, this, &PlaylistModel::endInsertItems);
        connect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeRemoved, this, &PlaylistModel::beginRemoveItems);
        connect(m_playlist.data(), &QMediaPlaylist::mediaRemoved, this, &PlaylistModel::endRemoveItems);
        connect(m_playlist.data(), &QMediaPlaylist::mediaChanged, this, &PlaylistModel::changeItems);
    }

    endResetModel();
}

bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role);
    m_data[index] = value;
    emit dataChanged(index, index);
    return true;
}

void PlaylistModel::beginInsertItems(int start, int end)
{
    qDebug("Inserting items %d, %d", start, end );
    m_data.clear();
    beginInsertRows(QModelIndex(), start, end);
}

void PlaylistModel::endInsertItems()
{
    endInsertRows();
}

void PlaylistModel::beginRemoveItems(int start, int end)
{
    qDebug("Deleting items %d, %d", start, end );
    m_data.clear();
    beginRemoveRows(QModelIndex(), start, end);
}

void PlaylistModel::endRemoveItems()
{
    endInsertRows();
}

void PlaylistModel::changeItems(int start, int end)
{
    m_data.clear();
    emit dataChanged(index(start,0), index(end,ColumnCount));
}

Qt::DropActions PlaylistModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
   Qt::ItemFlags flags;

   if (index.isValid())
       flags = Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   else
       flags = Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
   return flags;
}

QStringList PlaylistModel::mimeTypes() const
 {
     QStringList types;
     types << "video/mp4";
     return types;
 }

QMimeData *PlaylistModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << text;
        }
    }

    mimeData->setData("video/mp4", encodedData);
    return mimeData;
}

bool PlaylistModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
//    qDebug() << "Parent is: " << parent << endl;
//    qDebug() << "New row: " << row << endl;
//    if (action == Qt::IgnoreAction)
//        return true;
//         if (!data->hasFormat("video/mp4"))
//             return false;

////         if (column > 0)
////             return false;
              int beginRow;

              if (row != -1)
                  beginRow = row;
              else if (parent.isValid())
                       beginRow = parent.row();
              else
                       beginRow = rowCount(QModelIndex());

              QByteArray encodedData = data->data("video/mp4");
//              qDebug() << data-
              QDataStream stream(&encodedData, QIODevice::ReadOnly);
              QStringList newItems;
              int   rows = 0;

              while (!stream.atEnd()) {
                  QString text;
                  stream >> text;
                  newItems << text;
                  ++rows;
              }
              // Rows у нас всегда 1 т.к можем выбрать одну сука за раз
              // beginRow это новая позиция
              qDebug() << "InsertRows (beginRow, rows): " << beginRow << rows << endl;
//              insertRows(beginRow, rows, QModelIndex());


              // В поле text хранится название нового видоса
              foreach (QString text, newItems) {
//                  beginInsertRows(parent.parent(), beginRow, beginRow);
//                  endInsertRows();
//                  changeItems(beginRow, beginRow);
                  //insertRow(beginRow, text);
//                  QModelIndex idx = index(beginRow, 0, QModelIndex());
//                  qDebug() << idx.row();
//                  bool flg = setData(idx, text);
                  //qDebug() << flg;
                  beginRow++;
              }

              return true;
//    //beginInsertItems(row, row);
}
