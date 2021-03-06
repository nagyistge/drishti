#include <QtGui>
#include "ved.h"

#include "filter.h"


QStringList
VED::registerPlugin()
{
  QStringList regString;

  regString << "Vesselness Enhancement Diffusion";

  return regString;
}

void
VED::init()
{
  m_pvlFileManager = 0;
  m_lodFileManager = 0;
  m_clipPos.clear();
  m_clipNormal.clear();
  m_crops.clear();
  m_paths.clear();
  m_lutSize = 0;
  m_dataMin = m_dataMax = Vec(0,0,0);
  m_voxelScaling = Vec(1,1,1);
  m_previousDirectory = qApp->applicationDirPath();
  m_pruneLod = m_pruneX = m_pruneY = m_pruneZ = 0;
  m_pruneData.clear();
}

void VED::setPvlFileManager(VolumeFileManager *p) { m_pvlFileManager = p; }
void VED::setLodFileManager(VolumeFileManager *l) { m_lodFileManager = l; }
void VED::setClipInfo(QList<Vec> cp, QList<Vec> cn) { m_clipPos = cp; m_clipNormal = cn; }
void VED::setCropInfo(QList<CropObject> c) { m_crops = c; }
void VED::setSamplingLevel(int s) { m_samplingLevel = s; }
void VED::setDataLimits(Vec dmin, Vec dmax) { m_dataMin = dmin; m_dataMax = dmax; }
void VED::setVoxelScaling(Vec v) { m_voxelScaling = v; }
void VED::setPreviousDirectory(QString d) { m_previousDirectory = d; }
void VED::setLookupTable(int ls, QImage li) { m_lutSize = ls; m_lutImage = li; }

void VED::setPathInfo(QList<PathObject> c)
{
  m_paths.clear();
  for(int i=0; i<c.count(); i++)
    {
      if (c[i].useType() > 0) // add only if used for crop/blend
	m_paths << c[i];
    }
}

void
VED::setPruneData(int lod,
			 int px, int py, int pz,
			 QVector<uchar> p)
{
  m_pruneLod = lod;
  m_pruneX = px;
  m_pruneY = py;
  m_pruneZ = pz;
  m_pruneData = p;
}

void
VED::start()
{
  VolumeFileManager *fileManager;
  int depth, width, height;
  Vec dataMin, dataMax;
  Vec vscale;
  int samplingLevel;
  
  dataMin = m_dataMin;
  dataMax = m_dataMax;
  vscale = m_voxelScaling;
  samplingLevel = m_samplingLevel;

  if (samplingLevel > 1)
    {
      int od,ow,oh,ld,lw,lh;
      od = m_pvlFileManager->depth();
      ow = m_pvlFileManager->width();
      oh = m_pvlFileManager->height();
      ld = m_lodFileManager->depth();
      lw = m_lodFileManager->width();
      lh = m_lodFileManager->height();
      QStringList items;
      items << QString("%1 : %2 %3 %4").arg(samplingLevel).arg(lh).arg(lw).arg(ld);
      items << QString("1 : %1 %2 %3").arg(oh).arg(ow).arg(od);
      bool ok;
      QString sel;
      sel = QInputDialog::getItem(0,
				  "Select subsampling level",
				  "Sampling Level",
				  items,
				  0,
				  false,
				  &ok);
      if (ok)
	{
	  QStringList stxt = sel.split(" ", QString::SkipEmptyParts);
	  samplingLevel = stxt[0].toInt();
	}

      QMessageBox::information(0, "Sampling Level",
			       QString("Selected sampling level %1").\
			       arg(samplingLevel));
    }

  if(samplingLevel <= 1)
    fileManager = m_pvlFileManager;
  else
    {
      fileManager = m_lodFileManager;
      dataMin/=samplingLevel;
      dataMax/=samplingLevel;
      vscale*=samplingLevel;
    }

  depth = fileManager->depth();
  width = fileManager->width();
  height = fileManager->height();


  uchar *lut = new uchar[m_lutSize*256*256*4];
  memcpy(lut, m_lutImage.bits(), m_lutSize*256*256*4);

  VEDFilter filter;
  filter.start(fileManager,
	       depth, width, height,
	       dataMin, dataMax,
	       m_previousDirectory,
	       vscale,
	       m_clipPos, m_clipNormal,
	       m_crops, m_paths,
	       lut,
	       m_pruneLod, m_pruneX, m_pruneY, m_pruneZ,
	       m_pruneData);

  delete [] lut;
  lut = 0;
}

//-------------------------------
//-------------------------------
Q_EXPORT_PLUGIN2(ved, VED);
