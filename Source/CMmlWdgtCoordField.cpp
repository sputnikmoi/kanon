#include <cstdio>
#include <QtGui/QKeyEvent>
#include <QMimeData>
#include <QtGui/QDrag>
#include <QtWidgets/QMenu>
#include "CDlgCoordFieldSymbol.h"
#include "CFormula.h"
#include "CGlyph.h"
#include "CGuiMatrix.h"
#include "CMmlWdgtCoordField.h"
#include "CModelData.h"
#include "CWndMain.h"
#include "strutil.h"
#include "Util.h"

using std::string;
using std::vector;

/* METHOD *********************************************************************/
/**
  Ctor
@param      wndMain: Parent widget.
@param         type:
@param indexInModel: Index of coordinate/field within CModelData
*******************************************************************************/
CMmlWdgtCoordField::CMmlWdgtCoordField(CWndMain* wndMain, ECoordField type, size_t indexInModel)
	: CMmlWdgtBase(wndMain)
	, m_WndMain(wndMain)
	, m_Type(type)
	, m_IndexInModel(indexInModel)
{
	if (type == eField)
	{	// Overwrite CMmlWdgtBase setting
		setMinimumHeight(m_BaseFontPointSize + 16);
		setAcceptDrops(true);
	}
	updateFormula();
}

/* METHOD *********************************************************************/
/**
@return true for a coordinate (and not a field)
*******************************************************************************/
bool CMmlWdgtCoordField::isCoord() const
{
	return m_Type == eCoord;
}

/* METHOD *********************************************************************/
/**
@return "coordinate" | "field"
*******************************************************************************/
QString CMmlWdgtCoordField::strType() const
{
	return isCoord() ? "coordinate" : "field";
}

/* METHOD *********************************************************************/
/**
@return true for instances displaying "..." (to add new coords/fields).
*******************************************************************************/
bool CMmlWdgtCoordField::isMore() const
{
	return (isCoord() && m_IndexInModel >= model().numCoord()) || (!isCoord() && m_IndexInModel >= model().numField());
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtCoordField::updateVisibility()
{
	const bool coord(isCoord());
	if (isMore())
	{
		setVisible((coord && m_IndexInModel <= model().numCoord()) || (!coord && m_IndexInModel <= model().numField()));
		setVisible(true); // New expression invisibly small without this ??  /**/
		QString strTt("Click to add another " + strType() + " to the model.");
		strTt += "\n\nThis means that the wave vector \n"
			"dimension of one more " + strType() + " is to be determined,\n"
			"which requires one more term in the Lagrangian.";
		setToolTip(strTt);
	}
	else
	{
		setVisible(true);
		const string cmnt(model().glyphCoordField(m_IndexInModel, m_Type).comment());
		QString comment(trimWhite(cmnt).c_str());
		if (!comment.isEmpty())
		{
			comment += "\n";
		}
		if (coord)
		{
			setToolTip(comment + "- Click to edit the coordinate.\n"
				"- Drag the coordinate onto an operator (2 lines below)\n"
				"   to use the coordinate.");
		}
		else
		{
			setToolTip(comment + "- Click to edit the field.\n"
				"- Add the field to a term of the\n   Lagrangian below by drag and drop.");
		}
	}
}

/* METHOD *********************************************************************/
/**
  Updates expression according to CModelData.
*******************************************************************************/
void CMmlWdgtCoordField::updateFormula()
{
	m_Formula.clear();
	if (isMore())
	{
		m_Formula.add(new CGlyphNeutral(dot, true));
		m_Formula.add(new CGlyphNeutral(dot, true));
		m_Formula.add(new CGlyphNeutral(dot, true));
	}
	else
	{
		const CGlyphCoordField glyph(model().glyphCoordField(m_IndexInModel, m_Type));
		m_Formula.add(new CGlyphCoordField(glyph));
	}
	updateVisibility();
	updateMml();
}

/* METHOD *********************************************************************/
/**
  After drag/drop
*******************************************************************************/
bool CMmlWdgtCoordField::canPermuteFields() const
{
	return !isCoord() && !isMore();
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtCoordField::mouseMoveEvent(QMouseEvent*)
{
	if (!isMore())
	{
		killTimer(m_TimerId);
		m_TimerId = 0;
		// Start drag if there was substantial move.
		QByteArray dataThis;
		dataThis.setNum(unsigned(m_IndexInModel));
		QMimeData* mimeData{new QMimeData};
		mimeData->setData(m_Type == eCoord ? MimeFormat::Coord : MimeFormat::Field, dataThis);
		QDrag* drag{new QDrag(this)};
		drag->setMimeData(mimeData);
#if 1
		QPixmap pxmp;
		getPixmap(pxmp);
		drag->setPixmap(pxmp);
#else
		const QImage image(m_Type == eCoord ? ":/images/Coordinate.png" : ":/images/Field.png");
		drag->setPixmap(QPixmap::fromImage(image));
#endif
		drag->exec();
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtCoordField::mousePressEvent(QMouseEvent* ev)
{
	const bool isRightBtn{ev->button() == Qt::RightButton};
	if (isMore())
	{
		if (!isRightBtn)
		{
			m_TimerId = startTimer(200);
		}
	}
	else
	{
		if (isRightBtn && !isMore() && (!isCoord() || m_IndexInModel != 0))
		{
		    enum { idDelete };
			QMenu menu(this);
			menu.addAction("Delete " + strType())->setData(idDelete);
			if (QAction* act{menu.exec(QCursor::pos())}) switch (act->data().toInt())
			{
			case idDelete:
				if (isCoord())
				{
					guiMatrix().deleteCoord(m_IndexInModel);
				}
				else
				{
					guiMatrix().deleteField(m_IndexInModel);
				}
				break;
			default: break;
			}
		}
		else
		{
			m_TimerId = startTimer(200);
		}
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtCoordField::dragEnterEvent(QDragEnterEvent* ev)
{
	const QMimeData* md{ev->mimeData()};
	bool ok{};
	const int fieldIndex{md->data(MimeFormat::Field).toInt(&ok)};
	if (md->hasFormat(MimeFormat::Field) && !isMore() && int(m_IndexInModel) != fieldIndex)
	{
		ev->acceptProposedAction();
	}
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
void CMmlWdgtCoordField::dropEvent(QDropEvent* ev)
{
	if (ev->mimeData()->hasFormat(MimeFormat::Field))
	{	// Permutate fields
		bool ok;
		const size_t fxDst{m_IndexInModel};
		const size_t fxSrc{ev->mimeData()->data(MimeFormat::Field).toUInt(&ok)};
		if (ok)
		{	// Permute fields.
			const size_t numField{model().numField()};
			vector<size_t> permutation;
			for (size_t fx{}; fx < numField; fx++)
			{
				permutation.push_back(fx);
			}
			permutation[fxDst] = fxSrc;
			const int step{fxDst < fxSrc ? -1 : 1};
			for (size_t fx{fxSrc}; fx != fxDst; fx += step)
			{
				permutation[fx] = fx + step;
			}
#if 0
			fprintf(stderr, "Src %u, Dst %u\n", unsigned(fxSrc), unsigned(fxDst));
			for (auto fx : permutation) { fprintf(stderr, "%lu, ", fx); }
			fprintf(stderr, "\n");
#endif
			model().permuteFields(permutation);
			m_WndMain->permuteFields();
		}
	}
}

/* METHOD *********************************************************************/
/**
  Delayed mousePressEvent (to discriminate from drag-start).
*******************************************************************************/
void CMmlWdgtCoordField::timerEvent(QTimerEvent* ev)
{
	if (m_TimerId == ev->timerId())
	{
		killTimer(m_TimerId);
		m_TimerId = 0;
		// ToDo: Don't open dialog if this starts a drag !
		CDlgCoordFieldSymbol dlg(this, m_Type, m_IndexInModel);
		if (QDialog::Accepted == dlg.exec())
		{
			updateFormula();
			m_WndMain->updateMml();
			if (!isMore())
			{
				guiMatrix().determineCriticalDimension();
			}
			model().setDirty();
		}
	}
}

