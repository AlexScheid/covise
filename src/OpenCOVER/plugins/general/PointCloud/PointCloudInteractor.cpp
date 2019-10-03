/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#include <cover/coVRPluginSupport.h>
#include <cover/coVRMSController.h>
#include <cover/VRSceneGraph.h>
#include <cover/coInteractor.h>
#include <cover/coVRCommunication.h>
#include <PluginUtil/PluginMessageTypes.h>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Vec4>
#include <osg/Plane>

#include "PointCloud.h"
#include "PointCloudInteractor.h"

using namespace covise;
using namespace opencover;
using namespace osg;

PointCloudInteractor::PointCloudInteractor(coInteraction::InteractionType type, const char *name, coInteraction::InteractionPriority priority = Medium)
    : coTrackerButtonInteraction(type, name, priority)
    , m_selectedWithBox(false)
{
    fprintf(stderr, "\nPointCloudInteractor\n");
    selectedPointsGroup = new osg::Group();
    previewPointsGroup = new osg::Group();
	MovedPTSGroup = new osg::Group();
	cover->getObjectsRoot()->addChild(MovedPTSGroup.get());
    cover->getObjectsRoot()->addChild(selectedPointsGroup.get());
    cover->getObjectsRoot()->addChild(previewPointsGroup.get());
}


PointCloudInteractor::~PointCloudInteractor()
{
destroy();
}

bool PointCloudInteractor::destroy()
{
    cover->getObjectsRoot()->removeChild(selectedPointsGroup.get());
    cover->getObjectsRoot()->removeChild(previewPointsGroup.get());
	cover->getObjectsRoot()->removeChild(MovedPTSGroup.get());
    return true;
}

void
PointCloudInteractor::startInteraction()
{
    if (cover->debugLevel(3))
        fprintf(stderr, "\nPointCloudInteractor::startMove\n");

    // store hand mat
    Matrix initHandMat = cover->getPointerMat();
    // get postion and direction of pointer
    m_initHandPos = initHandMat.preMult(Vec3(0.0, 0.0, 0.0));
    m_initHandDirection = initHandMat.preMult(Vec3(0.0, 1.0, 0.0));
}


void
PointCloudInteractor::stopInteraction()
{
    if (cover->debugLevel(3))
        fprintf(stderr, "\nPointCloudInteractor::stopMove\n");

        while (previewPointsGroup->getNumChildren() > 0)
            previewPointsGroup->removeChild(0,1);

        previewPoints.clear();
        if (!m_deselection)
        {
			if (type == ButtonC)
			{
				if ((m_translation || m_rotation)&& selectedPoints.size() >= 2)
				{
					std::vector<Vec3> PtSVec;
					for (std::vector<pointSelection>::iterator i = selectedPoints.begin(); i < selectedPoints.end(); i++)
					{
						PtSVec.push_back(Vec3(i->file->pointSet[i->pointSetIndex].points[i->pointIndex].x, i->file->pointSet[i->pointSetIndex].points[i->pointIndex].y, i->file->pointSet[i->pointSetIndex].points[i->pointIndex].z));
					}
					Matrixd TraSnap;
					TraSnap.makeTranslate(PtSVec[1] - PtSVec[0]);
					MovePoints(TraSnap);
				}
				if ((m_rotation && selectedPoints.size() >= 4))
				{
					std::vector<Vec3> PtSVec;
					for (std::vector<pointSelection>::iterator i = selectedPoints.begin(); i < selectedPoints.end(); i++)
					{
						PtSVec.push_back(Vec3(i->file->pointSet[i->pointSetIndex].points[i->pointIndex].x, i->file->pointSet[i->pointSetIndex].points[i->pointIndex].y, i->file->pointSet[i->pointSetIndex].points[i->pointIndex].z));
					}
					Matrixd TraSnap;
					Vec3 Axis = PtSVec[1] - PtSVec[0];
					Vec3 From = PtSVec[2] - PtSVec[0];
					Vec3 To = PtSVec[3] - PtSVec[0];
					double Angle = acos((From * To) / (From.length() * To.length()));
					TraSnap.makeRotate(Angle, Axis);
					MovePoints(TraSnap);
				}
			}
			else
			{
				pointSelection bestPoint;
				bool hitPointSuccess = hitPoint(bestPoint);
				if (hitPointSuccess)
				{
					highlightPoint(bestPoint);
					if (m_rotation)
					{
						MovePoints(RotMat);
					}
					if (m_translation)
					{
						MovePoints(TraMat);
					}
					//if (m_translation || m_rotation)
					//{
					//	MovedPTS.push_back(bestPoint);
					//	int neededPts;
					//	if (m_translation)
					//	{
					//		neededPts = 2;
					//	}
					//	if (m_rotation)
					//	{
					//		if (m_files->size() > 1)
					//		{
					//			neededPts = 6;
					//		}
					//		else
					//		{
					//			neededPts = 3;
					//		}
					//	}
					//	if (MovedPTS.size() == neededPts)
					//	{
					//		//Vec3 CoorBestPoint = Vec3(bestPoint.file->pointSet[bestPoint.pointSetIndex].points[bestPoint.pointIndex].x,
					//		//	bestPoint.file->pointSet[bestPoint.pointSetIndex].points[bestPoint.pointIndex].y,
					//		//	bestPoint.file->pointSet[bestPoint.pointSetIndex].points[bestPoint.pointIndex].z);
					//		std::vector<Vec3> PtsCoord1, PtsCoord2;
					//		string filename = bestPoint.file->filename;
					//		for (std::vector<pointSelection>::iterator iter = MovedPTS.begin(); iter != MovedPTS.end(); iter++)
					//		{
					//			int i = iter->pointSetIndex;
					//			int j = iter->pointIndex;
					//			if (iter->file->filename == filename)
					//			{
					//				PtsCoord1.push_back(Vec3(iter->file->pointSet[i].points[j].x, iter->file->pointSet[i].points[j].y, iter->file->pointSet[i].points[j].z));
					//			}
					//			else
					//			{
					//				PtsCoord2.push_back(Vec3(iter->file->pointSet[i].points[j].x, iter->file->pointSet[i].points[j].y, iter->file->pointSet[i].points[j].z));
					//			}
					//		}
					//		CalcMoveVec(PtsCoord1, PtsCoord2, filename);
					//		MovedPTS.clear();
					//		selectedPointsGroup->removeChildren(0, selectedPointsGroup->getNumChildren());
					//		selectedPoints.clear();
					//	}
					//}
				}
			}
        }
        else
        {
            deselectPoint();
        }
}

bool PointCloudInteractor::hitPoint(pointSelection& bestPoint)
{
    bool hitPointSuccess = false;
    if (m_files)
    {
        Matrix currHandMat = cover->getPointerMat();
        Matrix invBase = cover->getInvBaseMat();

        currHandMat = currHandMat * invBase;
        Vec3 currHandBegin = currHandMat.preMult(Vec3(0.0, 0.0, 0.0));
        Vec3 currHandEnd = currHandMat.preMult(Vec3(0.0, 1.0, 0.0));
        Vec3 currHandDirection = currHandEnd - currHandBegin;
		if ((m_rotation && selectedPoints.size() >= 3)||(m_translation && selectedPoints.size() >= 1))
		{
			osg::MatrixTransform *MoTra = new osg::MatrixTransform();
			Vec3 newVec;
			bool success = false;
			if (m_rotation)
			{
				Vec3 SelPT1 = Vec3(selectedPoints[0].file->pointSet[selectedPoints[0].pointSetIndex].points[selectedPoints[0].pointIndex].x, selectedPoints[0].file->pointSet[selectedPoints[0].pointSetIndex].points[selectedPoints[0].pointIndex].y, selectedPoints[0].file->pointSet[selectedPoints[0].pointSetIndex].points[selectedPoints[0].pointIndex].z);
				Vec3 SelPT2 = Vec3(selectedPoints[1].file->pointSet[selectedPoints[1].pointSetIndex].points[selectedPoints[1].pointIndex].x, selectedPoints[1].file->pointSet[selectedPoints[1].pointSetIndex].points[selectedPoints[1].pointIndex].y, selectedPoints[1].file->pointSet[selectedPoints[1].pointSetIndex].points[selectedPoints[1].pointIndex].z);
				Vec3 SelPT3 = Vec3(selectedPoints[2].file->pointSet[selectedPoints[2].pointSetIndex].points[selectedPoints[2].pointIndex].x, selectedPoints[2].file->pointSet[selectedPoints[2].pointSetIndex].points[selectedPoints[2].pointIndex].y, selectedPoints[2].file->pointSet[selectedPoints[2].pointSetIndex].points[selectedPoints[2].pointIndex].z);
				Vec3 RotAxis = SelPT2 - SelPT1;
				if (OrgVec.length() == 0)
				{
					OrgVec = SelPT3;
				}
				Vec3 Ursprung = SelPT1 + RotAxis / RotAxis.length() * ((OrgVec - SelPT1)* RotAxis / RotAxis.length());
				double MovPointer = (currHandBegin - Ursprung).operator*(RotAxis /RotAxis.length());
				double a = -(MovPointer / (currHandDirection * RotAxis / RotAxis.length()));
				Vec3 Schnittpunkt = currHandBegin + currHandDirection * a;
				//Vec3 OrthToAxis1 = (OrgVec - SelPT1) - (RotAxis / RotAxis.length())*((OrgVec - SelPT1) * (RotAxis / RotAxis.length()));
				//Vec3 OrthToAxis2 = (Schnittpunkt - SelPT1) - (RotAxis / RotAxis.length())*((Schnittpunkt - SelPT1) * (RotAxis / RotAxis.length()));
				Vec3 DistNew = Schnittpunkt - Ursprung;
				Vec3 DistOld = OrgVec - Ursprung;
				Vec3 UpVec = DistOld ^ (OrgVec - SelPT1);
				double Angle = acos((DistNew * DistOld) / (DistNew.length() * DistOld.length()));
				if ((DistNew * DistOld) / (DistNew.length() * DistOld.length()) == -1)
				{
					Angle = PI;
				}
				if (DistNew * UpVec < 0)
				{
					Angle = -Angle;
				}
				Matrixd NewRot, NewTra, NewTra2, NewMat;
				NewRot.makeRotate(Angle, RotAxis);
				NewTra.makeTranslate(-Ursprung);
				NewTra2.makeTranslate(Ursprung);
				if (Angle != OldAngle)
				{
					RotMat.makeRotate(Angle - OldAngle, RotAxis);
					NewMat = NewTra * RotMat *NewTra2;
					MoTra->setMatrix(NewMat);
					RotMat = NewTra * NewRot * NewTra2;
					OldAngle = Angle;
				}
				newVec = NewRot.preMult(DistOld) + Ursprung;
			}
			if (m_translation)
			{
				
				Vec3 SelPT3 = Vec3(selectedPoints[0].file->pointSet[selectedPoints[0].pointSetIndex].points[selectedPoints[0].pointIndex].x, selectedPoints[0].file->pointSet[selectedPoints[0].pointSetIndex].points[selectedPoints[0].pointIndex].y, selectedPoints[0].file->pointSet[selectedPoints[0].pointSetIndex].points[selectedPoints[0].pointIndex].z);
				Vec3 Direct = SelPT3 - currHandBegin;
				Vec3 OrthToPointer = currHandDirection / currHandDirection.length() *(Direct * currHandDirection / currHandDirection.length());
				newVec = currHandBegin + OrthToPointer;

				if (TraMat.getTrans() != newVec)
				{
					cout << "Ungleich" << endl;
					TraMat.makeTranslate(newVec - TraMat.getTrans());
					MoTra->setMatrix(TraMat);
					TraMat.makeTranslate(newVec);
				}
				newVec = TraMat.preMult(SelPT3) - OrgVec;
			}
			osg::Node *oldDrawing, *newDrawing, *nGroupNode = new osg::Node();
			for (unsigned int i = 0; i < cover->getObjectsRoot()->getNumChildren(); i++)
			{
				nGroupNode = cover->getObjectsRoot()->getChild(i);
				if (nGroupNode->getName() == selectedPoints[0].file->filename)
				{
					oldDrawing = nGroupNode->asGroup()->getChild(0);
					break;
				}
			}
			MoTra->setName("TraMa");
			MoTra->addChild(oldDrawing);
			newDrawing = MoTra;
			newDrawing->setName("MovedGeode");
			nGroupNode->asGroup()->replaceChild(oldDrawing, newDrawing);
			bestPoint.pointSetIndex = selectedPoints[selectedPoints.size() - 1].pointSetIndex;
			bestPoint.pointIndex = selectedPoints[selectedPoints.size() - 1].file->pointSet->size + 1;
			bestPoint.file = selectedPoints[selectedPoints.size() - 1].file;
			hitPointSuccess = true;
			bestPoint.selectionIndex = selectionSetIndex;
			bestPoint.isBoundaryPoint = getSelectionIsBoundary();
			bestPoint.file->pointSet[bestPoint.pointSetIndex].points[bestPoint.pointIndex].x = newVec.x();
			bestPoint.file->pointSet[bestPoint.pointSetIndex].points[bestPoint.pointIndex].y = newVec.y();
			bestPoint.file->pointSet[bestPoint.pointSetIndex].points[bestPoint.pointIndex].z = newVec.z();
			highlightPoint(bestPoint, true);
		}
		else
		{
			double smallestDistance = FLT_MAX;
			for (std::vector<FileInfo>::const_iterator fit = m_files->begin(); fit != m_files->end(); fit++)
			{
				if (fit->pointSet)
				{
					//std::string fileName = fit->filename;
					//fprintf(stderr, "Testing Set %s \n", fileName.c_str());
					for (int i = 0; i < fit->pointSetSize; i++)
					{
						Vec3 center = Vec3((fit->pointSet[i].xmin + fit->pointSet[i].xmax) / 2, (fit->pointSet[i].ymin + fit->pointSet[i].ymax) / 2, (fit->pointSet[i].zmin + fit->pointSet[i].zmax) / 2);
						Vec3 corner = Vec3(fit->pointSet[i].xmin, fit->pointSet[i].ymin, fit->pointSet[i].zmin);
						Vec3 radiusVec = center - corner;
						float radius = 1.5 * radiusVec.length();

						if (hitPointSetBoundingSphere(currHandDirection, currHandBegin, center, radius))
						{
							for (int j = 0; j < fit->pointSet[i].size; j++)
							{
								Vec3 currentPoint = Vec3(fit->pointSet[i].points[j].x, fit->pointSet[i].points[j].y, fit->pointSet[i].points[j].z);
								double distance = LinePointMeasure(currentPoint, currHandBegin, currHandDirection);
								if (distance < smallestDistance)
								{
									smallestDistance = distance;
									bestPoint.pointSetIndex = i;
									bestPoint.pointIndex = j;
									bestPoint.file = &(*fit);
									hitPointSuccess = true;
									bestPoint.selectionIndex = selectionSetIndex;
									bestPoint.isBoundaryPoint = getSelectionIsBoundary();
								}
							}
						}
					}
				}
			}
			if (hitPointSuccess)
			{
				//dont select the same point twice
				for (std::vector<pointSelection>::iterator iter = selectedPoints.begin(); iter != selectedPoints.end(); iter++)
				{
					if (iter->pointSetIndex == bestPoint.pointSetIndex && iter->pointIndex == bestPoint.pointIndex)
					{
						hitPointSuccess = false;
					}
				}
				if (selectedPoints.size() == 2)
				{
					OldHandPos = currHandEnd;
					OldHandDirect = currHandDirection;
				}
			}
		}
    }
    return hitPointSuccess;
}

void PointCloudInteractor::doInteraction()
{
    if (cover->debugLevel(3))
        fprintf(stderr, "\nPointCloudInteractor::stopMove\n");
	if (type != ButtonC)
	{
		pointSelection bestPoint;
		bool hitPointSuccess = hitPoint(bestPoint);
		if (hitPointSuccess)
		{
			highlightPoint(bestPoint, true);
		}
	}
}

void  PointCloudInteractor::MovePoints(osg::Matrixd MoveMat)
{
	std::vector<FileInfo> Files;
	Files = *m_files;
	UpdatedFIVec.clear();
	MovedPTSGroup->removeChildren(0, MovedPTSGroup->getNumChildren());
	for (std::vector<FileInfo>::iterator File = Files.begin(); File != Files.end(); File++)
	{
		FileInfo FI;
		FI = *File;
		if (selectedPoints[0].file->filename == File->filename)
		{
			cout << "Filename: " << File->filename << endl;
			PointSet *PTSet;
			PTSet = File->pointSet;
			NodeInfo NI;
			osg::Matrixf *Move = new osg::Matrixf();
			for (int i = 0; i < PTSet->size; i++)
			{
				Vec3 PtCoord = Vec3(PTSet->points[i].x, PTSet->points[i].y, PTSet->points[i].z);
				PtCoord = MoveMat.preMult(PtCoord);
				if (m_translation)
				{
					PtCoord = PtCoord - OrgVec;
				}
				PTSet->points[i].x = PtCoord.x();
				PTSet->points[i].y = PtCoord.y();
				PTSet->points[i].z = PtCoord.z();
			}
			if (m_translation)
			{
				OrgVec = MoveMat.getTrans();
			}
			FI.nodes.clear();
			FI.pointSet = PTSet;
			PointCloudGeometry *drawable = new PointCloudGeometry(FI.pointSet);
			drawable->changeLod(1.f);
			drawable->setPointSize(1.f);
			drawable->setName("Drawable Gedrehte Punkte");
			Geode *nGeode = new Geode();
			nGeode->addDrawable(drawable);
			nGeode->setName(FI.filename);
			NI.node = nGeode;
			FI.nodes.push_back(NI);
			FI.filename = File->filename;
			MovedPTSGroup->setName("Moved Group");
			MovedPTSGroup->addChild(nGeode);
		}
		//PointSet *PTSet;
		//PTSet = File->pointSet;
		//NodeInfo NI;
		//FileInfo FI;
		//FI = *File;
		//osg::Matrixf *Move = new osg::Matrixf();
		//for (int i = 0; i < PTSet->size; i++)
		//{
		//	Vec3 PtCoord = Vec3(PTSet->points[i].x, PTSet->points[i].y, PTSet->points[i].z);
		//	PtCoord = MoveMat.preMult(PtCoord);
		//	if (m_translation)
		//	{
		//		PtCoord - OrgVec;
		//	}
		//	PTSet->points[i].x = PtCoord.x();
		//	PTSet->points[i].y = PtCoord.y();
		//	PTSet->points[i].z = PtCoord.z();
		//}
		//if (m_translation)
		//{
		//	OrgVec = MoveMat.getTrans();
		//}
		//FI.nodes.clear();
		//FI.pointSet = PTSet;
		//PointCloudGeometry *drawable = new PointCloudGeometry(FI.pointSet);
		//drawable->changeLod(1.f);
		//drawable->setPointSize(1.f);
		//drawable->setName("Drawable Gedrehte Punkte");
		//Geode *nGeode = new Geode();
		//nGeode->addDrawable(drawable);
		//nGeode->setName(FI.filename);
		//NI.node = nGeode;
		//FI.filename = File->filename;
		//cout << "FI Size: " << FI.pointSet->size << endl;
		//cout << "Punkt 1: x: " << FI.pointSet->points[0].x << " y: " << FI.pointSet->points[0].y << " z: " << FI.pointSet->points[0].z << endl;
		UpdatedFIVec.push_back(FI);
		cout << "Filename: " << FI.filename << endl;

	}
	updatePoints(&UpdatedFIVec);

}
void PointCloudInteractor::CalcMoveVec(std::vector<Vec3> SelPts1, std::vector<Vec3> SelPts2, string filename)
{
	std::vector<FileInfo> Files;
	Files = *m_files;
	UpdatedFIVec.clear();
	int i = 1;
	bool done = true;
	MovedPTSGroup->removeChildren(0, MovedPTSGroup->getNumChildren());
	for (std::vector<FileInfo>::iterator File = Files.begin(); File != Files.end(); File++)
	{
		PointSet *PTSet;
		PTSet = File->pointSet;
		NodeInfo NI;
		FileInfo FI;
		FI = *File;
		osg::Matrixf *Move = new osg::Matrixf();
		if (done && File->filename == filename)
		{
			osg::MatrixTransform *MoTra = new osg::MatrixTransform();
			osg::Node *oldDrawing, *newDrawing, *nGroupNode = new osg::Node();
			for (unsigned int i = 0; i < cover->getObjectsRoot()->getNumChildren(); i++)
			{
				nGroupNode = cover->getObjectsRoot()->getChild(i);
				if (nGroupNode->getName() == filename)
				{
					oldDrawing = nGroupNode->asGroup()->getChild(0);
					break;
				}
			}
			osg::Matrixf *Move = new osg::Matrixf();
			if (m_translation)
			{
				Vec3 CoorBestPoint;
				if (SelPts2.size() > 0)
				{
					CoorBestPoint = SelPts2[0] - SelPts1[0];
				}
				else
				{
					CoorBestPoint = SelPts1[1] - SelPts1[0];
				}
				Move = MakeTranslate(CoorBestPoint, Move, PTSet);
			}
			if (m_rotation)
			{
				osg::Matrixf *Rot = new osg::Matrixf(), *Tran = new osg::Matrixf();
				Vec3 Norm1 = (SelPts1[1] - SelPts1[0]).operator^(SelPts1[2]- SelPts1[1]);
				Vec3 Norm2, TransVec;
				if (SelPts2.size() > 1)
				{
					Norm2= (SelPts2[1] - SelPts2[0]).operator^(SelPts2[2] - SelPts2[1]);
				}
				else
				{
					Norm2 = Norm1;
				}
				//for (int i = 0; i < PTSet->size; i++)
				//{
				//	Vec3 PtVec = Vec3(PTSet->points[i].x, PTSet->points[i].y, PTSet->points[i].z);
				//	double xmin, xmax, ymin, ymax, zmin, zmax;
				//	double distance = (SelPts1[1] - SelPts1[0]).length();
				//	Vec3 VerVec1 = SelPts1[1] - (Norm1 / Norm1.length() * 0.1);
				//	Vec3 VerVec2 = SelPts1[1] + (Norm1 / Norm1.length() * 0.1);

				//	xmin = SelPts1[1].x - distance;
				//	xmax = SelPts1[1].x + distance;
				//	ymin = SelPts1[1].y - distance;
				//	ymax = SelPts1[1].y + distance;
				//	zmin = SelPts1[1].z - distance;
				//	zmax = SelPts1[1].z + distance;
				//	PTSet->points[i].x = PTSet->points[i].x - TraVec.x();
				//	PTSet->points[i].y = PTSet->points[i].y - TraVec.y();
				//	PTSet->points[i].z = PTSet->points[i].z - TraVec.z();
				//}
				TransVec = SelPts1[0] - SelPts2[0];
				Rot = MakeRotate(Norm1, Norm2, Rot, PTSet);
				Tran = MakeTranslate(TransVec, Tran, PTSet);
				Move = &Tran->operator*(*Rot);
			}
			Node *drawN = new osg::Node();
			drawN = MoTra;
			MoTra->setName("TraMa");
			MoTra->addChild(oldDrawing);
			MoTra->setMatrix(*Move);
			newDrawing = MoTra;
			newDrawing->setName("MovedGeode");
			nGroupNode->asGroup()->replaceChild(oldDrawing, newDrawing);
			done = false;		
		}
		else
		{
			MakeTranslate(Vec3(0, 0, 0), Move, PTSet);
		}
		FI.nodes.clear();
		FI.pointSet = PTSet;
		PointCloudGeometry *drawable = new PointCloudGeometry(FI.pointSet);
		drawable->changeLod(1.f);
		drawable->setPointSize(1.f);
		drawable->setName("Drawable Gedrehte Punkte");
		Geode *nGeode = new Geode();
		nGeode->addDrawable(drawable);
		nGeode->setName(FI.filename);
		NI.node = nGeode;
		FI.nodes.push_back(NI);
		FI.filename = File->filename;
		cout << "FI Size: " << FI.pointSet->size << endl;
		cout << "Punkt 1: x: " << FI.pointSet->points[0].x << " y: " << FI.pointSet->points[0].y << " z: " << FI.pointSet->points[0].z << endl;
		UpdatedFIVec.push_back(FI);
		cout << "Filename: " << FI.filename << endl;
		MovedPTSGroup->setName("Moved Group");
		MovedPTSGroup->addChild(nGeode);
		
	}
	updatePoints(&UpdatedFIVec);
}

osg::Matrixf*
PointCloudInteractor::MakeRotate(osg::Vec3f RotVec1, osg::Vec3f RotVec2, osg::Matrixf *RotMat, PointSet *PTSet)
{
	RotMat->makeRotate(RotVec1, RotVec2);
	//for (int i = 0; i < PTSet->size; i++)
	//{
	//	Vec3f PtsCoord = Vec3f(PTSet->points[i].x, PTSet->points[i].y, PTSet->points[i].z);
	//	PtsCoord = RotMat->getRotate().operator*(PtsCoord);
	//	PTSet->points[i].x = PtsCoord.x();
	//	PTSet->points[i].y = PtsCoord.y();
	//	PTSet->points[i].z = PtsCoord.z();
	//}
	return RotMat;
}

osg::Matrixf*
PointCloudInteractor::MakeTranslate(Vec3 TraVec, osg::Matrixf *TraMat, PointSet *PTSet)
{
	TraMat->makeTranslate(-TraVec);
	for (int i = 0; i < PTSet->size; i++)
	{
		PTSet->points[i].x = PTSet->points[i].x - TraVec.x();
		PTSet->points[i].y = PTSet->points[i].y - TraVec.y();
		PTSet->points[i].z = PTSet->points[i].z - TraVec.z();
	}
	return TraMat;
}

void
PointCloudInteractor::highlightPoint(pointSelection& selectedPoint, bool preview)
{
	Vec3 newSelectedPoint = Vec3(selectedPoint.file->pointSet[selectedPoint.pointSetIndex].points[selectedPoint.pointIndex].x,
		selectedPoint.file->pointSet[selectedPoint.pointSetIndex].points[selectedPoint.pointIndex].y,
		selectedPoint.file->pointSet[selectedPoint.pointSetIndex].points[selectedPoint.pointIndex].z);
	//fprintf(stderr,"Selected point has ID %d", selectedPoint.file->pointSet[selectedPoint.pointSetIndex].IDs[selectedPoint.pointIndex]);

    osg::Matrix *sphereTransformationMatrix = new osg::Matrix;
    sphereTransformationMatrix->makeTranslate(newSelectedPoint);

    osg::MatrixTransform *sphereTransformation = new osg::MatrixTransform;
    sphereTransformation->setMatrix(*sphereTransformationMatrix);
	sphereTransformation->setName("Sphere Transformation");
    selectedPoint.transformationMatrix = sphereTransformation;

    osg::Geode *sphereGeode = new osg::Geode;
    sphereTransformation->addChild(sphereGeode);
    osg::Sphere *selectedSphere = new osg::Sphere(newSelectedPoint,1.0f);
    osg::TessellationHints *hint = new osg::TessellationHints();
    hint->setDetailRatio(0.5);
    osg::StateSet* stateSet = VRSceneGraph::instance()->loadDefaultGeostate(osg::Material::AMBIENT_AND_DIFFUSE);
    osg::ShapeDrawable *selectedSphereDrawable = new osg::ShapeDrawable(selectedSphere, hint);
    osg::Material *selMaterial = new osg::Material();
    sphereGeode->addDrawable(selectedSphereDrawable);

    if (preview)
    {
        selMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4f(0.0, 0.6, 0.0, 1.0f));
        selMaterial->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4f(0.0, 0.6, 0.0, 1.0f));
        selMaterial->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4f(0.1f, 0.1f, 0.1f, 1.0f));
        selMaterial->setShininess(osg::Material::FRONT_AND_BACK, 10.f);
        selMaterial->setColorMode(osg::Material::OFF);
        if (previewPointsGroup->getNumChildren() == 0)
            previewPointsGroup->addChild(sphereTransformation);
        else
            previewPointsGroup->setChild(0,sphereTransformation);
        previewPoints.clear();
        previewPoints.push_back(selectedPoint);
    }
    else
    {
        selectedPointsGroup->addChild(sphereTransformation);
        if (!selectedPoint.isBoundaryPoint)
        {
        selMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4f(0.6, 0.0, 0.0, 1.0f));
        selMaterial->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4f(0.6, 0.0, 0.0, 1.0f));
        }
        else
        {
            selMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4f(0.0, 0.6, 0.0, 1.0f));
            selMaterial->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4f(0.0, 0.6, 0.0, 1.0f));
        }
        selMaterial->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4f(0.1f, 0.1f, 0.1f, 1.0f));
        selMaterial->setShininess(osg::Material::FRONT_AND_BACK, 10.f);
        selMaterial->setColorMode(osg::Material::OFF);
        //addSelectedPoint(newSelectedPoint);
        selectedPoints.push_back(selectedPoint);
        updateMessage(selectedPoints);
    }
    stateSet->setAttribute(selMaterial);
    selectedSphereDrawable->setStateSet(stateSet);
}


void
PointCloudInteractor::addSelectedPoint(Vec3 selectedPoint)
{
//send message to NurbsSurfacePlugin
cover->sendMessage(NULL, "NurbsSurface", PluginMessageTypes::NurbsSurfacePointMsg, sizeof(selectedPoint), &selectedPoint);
}

void PointCloudInteractor::updateMessage(vector<pointSelection> points)
{
    //send message to NurbsSurfacePlugin
    cover->sendMessage(NULL, "NurbsSurface", PluginMessageTypes::NurbsSurfacePointMsg, sizeof(points), &points);
    for (auto iter=points.begin(); iter !=points.end(); iter++)
    {
        if (iter->isBoundaryPoint)
            fprintf(stderr, "PointCloudInteractor::updateMessage sending boundary point!\n");
    }
}

double
PointCloudInteractor::LinePointMeasure(Vec3 center, Vec3 handPos, Vec3 handDirection)
{
    handDirection.normalize();
    Vec3 pMinusC = handPos - center;
    double b = handDirection * pMinusC;
    double c = sqrt(pMinusC * pMinusC - b * b);
    //double d = pMinusC.length();
    if (pMinusC.length()==0.)
        return 0.;
    
    double d = c * pMinusC.length();
    return d;
}

double
PointCloudInteractor::LinePointDistance(Vec3 center, Vec3 handPos, Vec3 handDirection)
{
    handDirection.normalize();
    Vec3 pMinusC = handPos - center;
    double b = handDirection * pMinusC;
    double c = sqrt(pMinusC * pMinusC - b * b);
    //double d = pMinusC.length();
    return c;

}

bool PointCloudInteractor::hitPointSetBoundingSphere(osg::Vec3 handDir, osg::Vec3 handPos, Vec3 center, float radius)
{
    float distance = LinePointDistance(center,handPos, handDir);
    if (distance<=radius)
    {
        return true;
    }
    return false;
}

bool PointCloudInteractor::hitPointSet(osg::Vec3 handDir, osg::Vec3 handPos, PointSet *pointset)
{
    osg::Vec3 contact;
    handDir.normalize();
    // build Plane
    //osg::Plane plane = osg::Plane(0.,0.,1.,-pointset.xmin);
    osg::Vec3 coord[6]= { 
        Vec3(pointset->xmin,0.,0.),
        Vec3(pointset->xmax,0.,0.),
        Vec3(0.,pointset->ymin,0.),
        Vec3(0.,pointset->ymax,0.),
        Vec3(0.,0.,pointset->zmin), 
        Vec3(0.,0.,pointset->zmax)
    };

    osg::Vec3 normal[6] = {
        Vec3(1.,0.,0.),
        Vec3(1.,0.,0.),
        Vec3(0.,1.,0.),
        Vec3(0.,1.,0.),
        Vec3(0.,0.,1.),
        Vec3(0.,0.,1.)
    };

    for (int i=0; i<6;i++)
    {
        if ( abs(normal[i] * handDir ) >= 0.0001)
        {
            float d = normal[i] * coord[i];
            float t = (d - normal[i] * handPos)/(normal[i] * handDir);
            osg::Vec3 newRay = handDir * t;
            contact = handPos + newRay;

            if (((i==0 || i==1) && (contact.z()<pointset->zmax && contact.z()>pointset->zmin)
                        && (contact.y()<pointset->ymax && contact.y()>pointset->ymin)) ||
                    ((i==2 || i==3) && (contact.x()<pointset->xmax && contact.x()>pointset->xmin)
                     && (contact.z()<pointset->zmax && contact.z()>pointset->zmin)) ||
                    ((i==4 || i==5) && (contact.x()<pointset->xmax && contact.x()>pointset->xmin)
                     && (contact.y()<pointset->ymax && contact.y()>pointset->ymin)) )
            {
                fprintf(stderr, "\nhitPointSet !\n");
                return true;
            }
        }
    }
    return false;
}

void PointCloudInteractor::resize()
{
    osg::Vec3 wpoint1 = osg::Vec3(0, 0, 0);
    osg::Vec3 wpoint2 = osg::Vec3(0, 0, 1);
    osg::Vec3 opoint1 = wpoint1 * cover->getInvBaseMat();
    osg::Vec3 opoint2 = wpoint2 * cover->getInvBaseMat();

    //distance formula
    osg::Vec3 wDiff = wpoint2 - wpoint1;
    osg::Vec3 oDiff = opoint2 - opoint1;
    double distWld = wDiff.length();
    double distObj = oDiff.length();

    //controls the sphere size
    double scaleFactor = sphereSize * distObj / distWld;
    //scaleFactor = 1.1f;

    // scale all selected points
    for (std::vector<pointSelection>::iterator iter = selectedPoints.begin(); iter !=selectedPoints.end(); iter++)
    {
        osg::Matrix sphereMatrix = iter->transformationMatrix->getMatrix();
        Vec3 translation = sphereMatrix.getTrans();
        sphereMatrix.makeScale(scaleFactor, scaleFactor, scaleFactor);
        sphereMatrix.setTrans(translation);
        iter->transformationMatrix->setMatrix(sphereMatrix);
    }
    
    //scale the preview point(s)
    for (std::vector<pointSelection>::iterator iter = previewPoints.begin(); iter !=previewPoints.end(); iter++)
    {
        osg::Matrix sphereMatrix = iter->transformationMatrix->getMatrix();
        Vec3 translation = sphereMatrix.getTrans();
        sphereMatrix.makeScale(scaleFactor, scaleFactor, scaleFactor);
        sphereMatrix.setTrans(translation);
        iter->transformationMatrix->setMatrix(sphereMatrix);
    }
}

bool PointCloudInteractor::deselectPoint()
{
    bool hitPointSuccess = false;
    if (m_files)
    {
        Matrix currHandMat = cover->getPointerMat();
        Matrix invBase = cover->getInvBaseMat();
		
        currHandMat = currHandMat * invBase;

        Vec3 currHandBegin = currHandMat.preMult(Vec3(0.0, 0.0, 0.0));
        Vec3 currHandEnd = currHandMat.preMult(Vec3(0.0, 1.0, 0.0));
        Vec3 currHandDirection = currHandEnd - currHandBegin;

        double smallestDistance = FLT_MAX;
        std::vector<pointSelection>::iterator deselectionPoint;

        for (std::vector<pointSelection>::iterator iter = selectedPoints.begin(); iter !=selectedPoints.end(); iter++)
        {
            int i = iter->pointSetIndex;
            int j = iter->pointIndex;
            Vec3 currentPoint = Vec3(iter->file->pointSet[i].points[j].x,iter->file->pointSet[i].points[j].y,iter->file->pointSet[i].points[j].z);
            double distance = LinePointDistance(currentPoint, currHandBegin, currHandDirection);
            if (distance<smallestDistance)
            {
                smallestDistance=distance;
                deselectionPoint = iter;
                hitPointSuccess = true;
            }                
        }
        if (hitPointSuccess)
        {
            selectedPointsGroup->removeChild(deselectionPoint->transformationMatrix);
            selectedPoints.erase(deselectionPoint);
        }
    }
    updateMessage(selectedPoints);
    return hitPointSuccess;
}

void PointCloudInteractor::setTranslation(bool translation)
{
	if (translation)
		m_translation = true;
	else
		m_translation = false;
}

std::vector<pointSelection>& PointCloudInteractor::getSelectedPtS()
{
	return selectedPoints;
}

void PointCloudInteractor::setSelectedPts(std::vector<pointSelection>& SelPts)
{

	selectedPoints = SelPts;
}

void PointCloudInteractor::setRotation(bool rotation)
{
	if (rotation)
	{
		m_rotation = true;
	}
	else
		m_rotation = false;
}

void PointCloudInteractor::setDeselection(bool deselection)
{
    if (deselection)
        m_deselection = true;
    else
        m_deselection = false;
}

void PointCloudInteractor::setSelectionSetIndex(int selectionSet)
{
    selectionSetIndex=selectionSet;
}

void PointCloudInteractor::setSelectionIsBoundary(bool selectionIsBoundary)
{
    m_selectionIsBoundary=selectionIsBoundary;
}

bool PointCloudInteractor::getSelectionIsBoundary()
{
    return m_selectionIsBoundary;
}
