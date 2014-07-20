/* qv4l2: a control panel controlling v4l2 devices.
 *
 * Copyright (C) 2006 Hans Verkuil <hverkuil@xs4all.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef QV4L2_H
#define QV4L2_H

#include <config.h>

#include <QMainWindow>
#include <QTabWidget>
#include <QSignalMapper>
#include <QLabel>
#include <QGridLayout>
#include <QSocketNotifier>
#include <QImage>
#include <QFileDialog>
#include <map>
#include <vector>

// Must come before cv4l-helpers.h
#include <libv4l2.h>

#include "cv4l-helpers.h"
#include "raw2sliced.h"
#include "capture-win.h"

class QComboBox;
class QSpinBox;
class GeneralTab;
class VbiTab;
class QCloseEvent;
class CaptureWin;

typedef std::vector<unsigned> ClassIDVec;
typedef std::map<unsigned, ClassIDVec> ClassMap;
typedef std::map<unsigned, struct v4l2_query_ext_ctrl> CtrlMap;
typedef std::map<unsigned, QWidget *> WidgetMap;

enum {
	CTRL_UPDATE_ON_CHANGE = 0x10,
	CTRL_DEFAULTS,
	CTRL_REFRESH,
	CTRL_UPDATE
};

enum CapMethod {
	methodRead,
	methodMmap,
	methodUser
};

enum RenderMethod {
	QV4L2_RENDER_GL,
	QV4L2_RENDER_QT
};

struct buffer {
	unsigned planes;
	void   *start[VIDEO_MAX_PLANES];
	size_t  length[VIDEO_MAX_PLANES];
	bool clear;
};

#define CTRL_FLAG_DISABLED	(V4L2_CTRL_FLAG_READ_ONLY | \
				 V4L2_CTRL_FLAG_INACTIVE | \
				 V4L2_CTRL_FLAG_GRABBED)

class CaptureWin;

class ApplicationWindow: public QMainWindow, cv4l_fd
{
	Q_OBJECT

	friend CaptureWin;
public:
	ApplicationWindow();
	virtual ~ApplicationWindow();

private slots:
	void closeDevice();
	void closeCaptureWin();

public:
	void setDevice(const QString &device, bool rawOpen);

	// capturing
private:
	CaptureWin *m_capture;

	bool startCapture();
	void stopCapture();
	void newCaptureWin();
	void startAudio();
	void stopAudio();

	bool m_clear[64];
	cv4l_fmt m_capSrcFormat;
	cv4l_fmt m_capDestFormat;
	unsigned char *m_frameData;
	unsigned m_nbuffers;
	struct v4lconvert_data *m_convertData;
	bool m_mustConvert;
	CapMethod m_capMethod;
	bool m_makeSnapshot;
	RenderMethod m_renderMethod;

private slots:
	void capStart(bool);
	void makeFullScreen(bool);
	void capFrame();
	void ctrlEvent();
	void snapshot();
	void capVbiFrame();
	void saveRaw(bool);
	void setRenderMethod(bool);
	void setBlending(bool);
	void traceIoctls(bool);
	void changeAudioDevice();

	// gui
private slots:
	void opendev();
	void openrawdev();
	void ctrlAction(int);
	void openRawFile(const QString &s);
	void rejectedRawFile();
	void setAudioBufferSize();
	void enableScaling(bool enable);
	void updatePixelAspectRatio();
	void updateCropping();
	void updateColorspace();
	void updateDisplayColorspace();
	void clearBuffers();
	void about();

public:
	virtual void error(const QString &text);
	void error(int err);
	void errorCtrl(unsigned id, int err);
	void errorCtrl(unsigned id, int err, long long v);
	void errorCtrl(unsigned id, int err, const QString &v);
	void info(const QString &info);
	virtual void closeEvent(QCloseEvent *event);
	QAction *m_resetScalingAct;
	QAction *m_useBlendingAct;
	QAction *m_snapshotAct;
	QAction *m_showFramesAct;

private:
	void addWidget(QGridLayout *grid, QWidget *w, Qt::Alignment align = Qt::AlignLeft);
	void addLabel(QGridLayout *grid, const QString &text, Qt::Alignment align = Qt::AlignLeft)
	{
		addWidget(grid, new QLabel(text, parentWidget()), align);
	}
	void fixWidth(QGridLayout *grid);
	void addTabs(int size[]);
	void finishGrid(QGridLayout *grid, unsigned ctrl_class);
	void addCtrl(QGridLayout *grid, const struct v4l2_query_ext_ctrl &qec);
	void updateCtrl(unsigned id);
	void updateCtrlRange(unsigned id, __s32 val);
	void subscribeCtrlEvents();
	void refresh(unsigned ctrl_class);
	void refresh();
	void makeSnapshot(unsigned char *buf, unsigned size);
	void setDefaults(unsigned ctrl_class);
	int getVal(unsigned id);
	long long getVal64(unsigned id);
	QString getString(unsigned id);
	void setVal(unsigned id, int v);
	void setVal64(unsigned id, long long v);
	void setString(unsigned id, const QString &v);
	QString getCtrlFlags(unsigned flags);
	void setWhat(QWidget *w, unsigned id, const QString &v);
	void setWhat(QWidget *w, unsigned id, long long v);
	void updateVideoInput();
	void updateVideoOutput();
	void updateAudioInput();
	void updateAudioOutput();
	void updateStandard();
	void updateFreq();
	void updateFreqChannel();
	bool showFrames();

	cv4l_queue m_queue;

	const double m_pxw;
	const int m_minWidth;
	const int m_vMargin;
	const int m_hMargin;
	int m_maxw[4];
	int m_maxh;
	int m_increment;
	GeneralTab *m_genTab;
	VbiTab *m_vbiTab;
	QAction *m_capStartAct;
	QAction *m_saveRawAct;
	QAction *m_useGLAct;
	QAction *m_showAllAudioAct;
	QAction *m_audioBufferAct;
	QAction *m_scalingAct;
	QAction *m_makeFullScreenAct;
	QString m_filename;
	QSignalMapper *m_sigMapper;
	QTabWidget *m_tabs;
	QSocketNotifier *m_capNotifier;
	QSocketNotifier *m_ctrlNotifier;
	QImage *m_capImage;
	int m_row, m_col, m_cols;
	CtrlMap m_ctrlMap;
	WidgetMap m_widgetMap;
	WidgetMap m_sliderMap;
	ClassMap m_classMap;
	int m_vbiSize;
	unsigned m_vbiWidth;
	unsigned m_vbiHeight;
	struct vbi_handle m_vbiHandle;
	unsigned m_frame;
	unsigned m_lastFrame;
	unsigned m_fps;
	struct timeval m_tv;
	struct timeval m_totalAudioLatency;
	QFile m_saveRaw;
};

extern ApplicationWindow *g_mw;

class SaveDialog : public QFileDialog
{
	Q_OBJECT

public:
	SaveDialog(QWidget *parent, const QString &caption) :
		QFileDialog(parent, caption), m_buf(NULL) {}
	virtual ~SaveDialog() {}
	bool setBuffer(unsigned char *buf, unsigned size);

public slots:
	void selected(const QString &s);

private:
	unsigned char *m_buf;
	unsigned m_size;
};

#endif
