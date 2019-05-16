#include "stdafx.h"
#include "Python.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "tisudshl.h"
#include "structmember.h"

using namespace DShowLib;

const char a[] = "PowerC LTD";


namespace gpowerdata {
	const int framebuffersize{ 1 };
}

struct camobj {
	Grabber * grabg1;
	long long camsn;
	std::list<std::string> avalvidformat;
	std::string currentresolution;
	int focus;
	int exposition;
	int gain;
	BYTE* pBuf[1];
	tFrameHandlerSinkPtr pSink;
	FrameTypeInfo info;
	IplImage *frame;
};

camobj gcam;

bool caminit(camobj * camp) {
	if (camp->grabg1 != NULL) {
		delete camp->grabg1;
	}
	camp->grabg1 = new Grabber();
	Grabber::tVidCapDevListPtr devlist = camp->grabg1->getAvailableVideoCaptureDevices();
	if (!devlist->empty())
	{
		for (Grabber::tVidCapDevListPtr::value_type::iterator it = devlist->begin(); it != devlist->end(); it++)
		{
			std::cout << it->c_str() << std::endl;
			it->getSerialNumber(camp->camsn);
		}
		camp->grabg1->openDev(devlist->front());
		Grabber::tVidFmtListPtr vformat;
		vformat = camp->grabg1->getAvailableVideoFormats();
		for (Grabber::tVidFmtListPtr::value_type::iterator it = vformat->begin(); it != vformat->end(); it++)
		{
			std::string tms;
			tms.assign((it->c_str()));
			camp->avalvidformat.push_back(tms);
		}
	}
	else
	{
		return false;
	}
}

PyObject * getcaminfo(void){
	caminit(&gcam);
	PyObject* pl = PyList_New(gcam.avalvidformat.size());
	std::list<std::string>::iterator it;
	int listpos{ 0 };
	for (it = gcam.avalvidformat.begin(); it != gcam.avalvidformat.end(); ++it) {
		PyList_SetItem(pl, listpos, (Py_BuildValue("s", it->c_str())));
		listpos++;
	}
	return pl;
}

void disabletonemapping(camobj * camp)
{
	DShowLib::tIVCDPropertyItemsPtr p_properties = gcam.grabg1->getAvailableVCDProperties();
	tVCDPropertyItemArray pAr1 = p_properties->getItems();
	for (unsigned int iid = 0; iid < pAr1.size(); iid++)
	{
		GUID guid1 = pAr1.at(iid)->getItemID();
		GUID elemIDSwitch2 = VCDElement_Auto;
		if (pAr1.at(iid)->getName() == "Tone Mapping")
		{
			tIVCDPropertyElementPtr pFoundElementPush2 = p_properties->findElement(guid1, elemIDSwitch2);
			if (pFoundElementPush2 != 0)
			{
				tIVCDSwitchProperty2Ptr pTMapSw = 0;
				pFoundElementPush2->getInterfacePtr(pTMapSw);
				if (pTMapSw != 0)
				{
					std::cout << "Try to disable tona mapping" << std::endl;
					std::cout << "Current mapping switch state: " << pTMapSw->getSwitch() << std::endl;
					pTMapSw->setSwitch(false);
					std::cout << "Mapping switch switched to: " << pTMapSw->getSwitch() << std::endl;
				}

			}
		}
	}
}

void camsetresfocex(camobj * camp)
{
	gcam.grabg1->setVideoFormat(camp->currentresolution);


	DShowLib::tIVCDPropertyItemsPtr p_properties = camp->grabg1->getAvailableVCDProperties();
	
	if (p_properties != 0)
	{
		tIVCDPropertyItemPtr pExposureItem = p_properties->findItem(VCDID_Exposure);
		if (pExposureItem != 0)
		{
			tIVCDSwitchPropertyPtr pAutoEx = 0;
			tIVCDRangePropertyPtr pValEx = 0;
			tIVCDPropertyElementPtr pExposureValueElement = pExposureItem->findElement(VCDElement_Value);
			tIVCDPropertyElementPtr pExposureAutoElement = pExposureItem->findElement(VCDElement_Auto);

			if (pExposureAutoElement != 0)
			{
				pExposureAutoElement->getInterfacePtr(pAutoEx);
				if (pAutoEx != 0)
				{
					std::cout << "Current Auto exposure switch is: " << pAutoEx->getSwitch() << std::endl;
					pAutoEx->setSwitch(false);
					std::cout << "Auto exposure switch set to: " << pAutoEx->getSwitch() << std::endl;
				}
			}
			if (pExposureValueElement != 0)
			{
				pExposureValueElement->getInterfacePtr(pValEx);
				if (pAutoEx != 0)
				{
					std::cout << "Current exposure value is: " << pValEx->getValue() << std::endl;
					std::cout << "Current exposure Max is: " << pValEx->getRangeMax() << std::endl;
					std::cout << "Current exposure Min is: " << pValEx->getRangeMin() << std::endl;
					std::cout << "Current exposure delta is: " << pValEx->getDelta() << std::endl;
					pValEx->setValue(camp->exposition);
					std::cout << "exposure value set to: " << pValEx->getValue() << std::endl;
				}
			}

		}

		tIVCDPropertyItemPtr pGainItem = p_properties->findItem(VCDID_Gain);
		if (pGainItem != 0)
		{
			tIVCDSwitchPropertyPtr pAutoGain = 0;
			tIVCDRangePropertyPtr pValGain = 0;
			tIVCDPropertyElementPtr pGainValueElement = pGainItem->findElement(VCDElement_Value);
			tIVCDPropertyElementPtr pGainAutoElement = pGainItem->findElement(VCDElement_Auto);

			if (pGainAutoElement != 0)
			{
				pGainAutoElement->getInterfacePtr(pAutoGain);
				if (pAutoGain != 0)
				{
					std::cout << "Current Auto gain switch is: " << pAutoGain->getSwitch() << std::endl;
					pAutoGain->setSwitch(false);
					std::cout << "Auto gain switch set to: " << pAutoGain->getSwitch() << std::endl;
				}
			}
			if (pGainValueElement != 0)
			{
				pGainValueElement->getInterfacePtr(pValGain);
				if (pAutoGain != 0)
				{
					std::cout << "Current gain value is: " << pValGain->getValue() << std::endl;
					std::cout << "Current gain Max is: " << pValGain->getRangeMax() << std::endl;
					std::cout << "Current gain Min is: " << pValGain->getRangeMin() << std::endl;
					std::cout << "Current gain delta is: " << pValGain->getDelta() << std::endl;
					pValGain->setValue(camp->gain);
					std::cout << "Gain value set to: " << pValGain->getValue() << std::endl;
				}
			}

		}

		tIVCDPropertyElementPtr FocusVal = p_properties->findElement(VCDID_Focus, VCDElement_Value);
		if (FocusVal != 0)
		{
			tIVCDRangePropertyPtr pRangeFocus = 0;
			FocusVal->getInterfacePtr(pRangeFocus);
			if (pRangeFocus != 0)
			{
				std::cout << "Current Focus value is: " << pRangeFocus->getValue() << std::endl;
				std::cout << "Current Focus Max is: " << pRangeFocus->getRangeMax() << std::endl;
				std::cout << "Current Focus Min is: " << pRangeFocus->getRangeMin() << std::endl;
				std::cout << "Current Focus delta is: " << pRangeFocus->getDelta() << std::endl;
				pRangeFocus->setValue(camp->focus);
				std::cout << "Focus value set to: " << pRangeFocus->getValue() << std::endl;
			}
		}

		disabletonemapping(camp);
	}
}


void pushfocus(void)
{
	DShowLib::tIVCDPropertyItemsPtr p_properties = gcam.grabg1->getAvailableVCDProperties();
	tIVCDPropertyElementPtr pFocusButton = p_properties->findElement(VCDID_Focus, VCDElement_OnePush);
	if (pFocusButton != 0)
	{
		tIVCDButtonPropertyPtr pAutoFocus = 0;
		pFocusButton->getInterfacePtr(pAutoFocus);
		if (pAutoFocus != 0)
		{
			std::cout << "Start auto Focus ...." << std::endl;
			pAutoFocus->push();
		}
	}
}

void pushwballance(void)
{
	DShowLib::tIVCDPropertyItemsPtr p_properties = gcam.grabg1->getAvailableVCDProperties();
	tIVCDPropertyElementPtr pWBPushElement = p_properties->findElement(VCDID_WhiteBalance, VCDElement_OnePush);
	if (pWBPushElement != 0)
	{
		tIVCDButtonPropertyPtr pWBOnePush = 0;
		pWBPushElement->getInterfacePtr(pWBOnePush);
		if (pWBOnePush != 0)
		{
			std::cout << "Get WB" << std::endl;
			pWBOnePush->push();
		}
	}
}

int getfocusdistance(void)
{
	std::cout << "Read current focus range" << std::endl;
	DShowLib::tIVCDPropertyItemsPtr p_properties = gcam.grabg1->getAvailableVCDProperties();
	tIVCDPropertyElementPtr FocusVal = p_properties->findElement(VCDID_Focus, VCDElement_Value);
	if (FocusVal != 0)
	{
		tIVCDRangePropertyPtr pRangeFocus = 0;
		FocusVal->getInterfacePtr(pRangeFocus);
		if (pRangeFocus != 0)
		{
			std::cout << "Focus value is: " << pRangeFocus->getValue() << std::endl;
			return pRangeFocus->getValue();
		}
	}
	return -1;
}

void * preparecam(void) {
	gcam.pSink = FrameHandlerSink::create(eRGB32, 1);
	gcam.pSink->setSnapMode(true);
	gcam.grabg1->setSinkType(gcam.pSink);
	if (!gcam.grabg1->prepareLive(false))
	{
		std::cerr << "Could not render the VideoFormat into a eRGB32 sink.";
		return NULL;
	}
	gcam.pSink->getOutputFrameType(gcam.info);
	// Allocate 1 image buffers of the above calculate buffer size.
	for (int i = 0; i < gpowerdata::framebuffersize; ++i)
	{
		gcam.pBuf[i] = new BYTE[gcam.info.buffersize];
	}

	cv::Size sz = cv::Size(int(gcam.info.dim.cx), int(gcam.info.dim.cy));
	int channel{ 4 };
	gcam.frame = cvCreateImage(sz, IPL_DEPTH_8U, channel);

	tMemBufferCollectionPtr pCollection = MemBufferCollection::create(gcam.info, 1, gcam.pBuf);
	if (pCollection == 0 || !gcam.pSink->setMemBufferCollection(pCollection))
	{
		std::cerr << "Could not set the new MemBufferCollection, because types do not match.";
		for (int i = 0; i < gpowerdata::framebuffersize; ++i)
		{
			delete gcam.pBuf[i];
		}
		return NULL;
	}

	//gcam.grabg1->startLive(false);

	Py_RETURN_NONE;
}

PyObject * setresolution(PyObject * self, PyObject * args) {
	char * resstrc;
	int focus;
	int exp;
	int gain;
	if (!PyArg_ParseTuple(args, "siii", &resstrc, &focus, &exp,&gain)) {
		return NULL;
	}
	/* Do something interesting here. */
	gcam.currentresolution.assign(resstrc);
	gcam.focus = focus;
	gcam.exposition = exp;
	gcam.gain = gain;
	//std::cout << gcam.currentresolution << gcam.focus << gcam.exposition << std::endl;

	camsetresfocex(&gcam);
	preparecam();

	Py_RETURN_NONE;
}

PyObject * stopvideo()
{
	gcam.grabg1->stopLive();
	Py_RETURN_NONE;
}

PyObject * startvideo()
{
	gcam.grabg1->startLive();
	Py_RETURN_NONE;
}

PyObject * makeautofocus()
{
	pushfocus();
	Py_RETURN_NONE;
}

PyObject * makewb()
{
	pushwballance();
	Py_RETURN_NONE;
}

PyObject * getfocusval()
{
	return PyLong_FromLong(getfocusdistance());
}


PyObject * getframe() {
	gcam.pSink->getOutputFrameType(gcam.info);
	cv::Mat d;
	
	gcam.pSink->snapImages(1);
	

	gcam.frame->imageData = (char *)gcam.pBuf[0];
	d.create(int(gcam.info.dim.cy), int(gcam.info.dim.cx), CV_8UC4);
	d.data = (uchar *)gcam.frame->imageData;
	std::cout << gcam.info.dim.cx << std::endl;
	//int step = ((sz.width));
	//cvSetData(frame, (char *)pBuf[0], step);
	//c = cv::cvarrToMat(frame);
	//cv::flip(d, e, 0);
	//f = e.clone();
	//std::cout << f.cols << std::endl;
	//cv::Mat s;

	PyObject* pl = PyList_New(3);
	std::string retmessage;
	long imsizel = 0;
	if (d.data == NULL) {
		retmessage = "error make test";
	}
	else {
		retmessage = "Test OK";
		imsizel = d.total() * d.elemSize();
	}
	PyList_SetItem(pl, 0, PyUnicode_FromString(retmessage.c_str()));

	PyObject *bar;
	bar = PyByteArray_FromStringAndSize((const char *)d.data, imsizel);

	PyObject * imsize;
	imsize = PyLong_FromLong(imsizel);

	PyList_SetItem(pl, 1, bar);
	PyList_SetItem(pl, 2, imsize);

	return pl;

}

cv::Mat inttest(void) {
	cv::Mat d, e, f;
	Grabber *grabber1 = new Grabber();
	tFrameHandlerSinkPtr dt;
	long long camsn;
	Grabber::tVidCapDevListPtr devlist = grabber1->getAvailableVideoCaptureDevices();
	if (!devlist->empty())
	{

		for (Grabber::tVidCapDevListPtr::value_type::iterator it = devlist->begin(); it != devlist->end(); it++)
		{
			std::cout << it->c_str() << std::endl;
			it->getSerialNumber(camsn);
			//it->getDisplayName();
			std::cout << camsn << std::endl;
		}
		grabber1->openDev(devlist->front());


		DShowLib::tIVCDPropertyItemsPtr p_properties = grabber1->getAvailableVCDProperties();
		tVCDPropertyItemArray pAr1 = p_properties->getItems();
		for (unsigned int iid = 0; iid < pAr1.size(); iid++)
		{
			GUID guid1 = pAr1.at(iid)->getItemID();
			std::cout << guid1.Data1 << std::endl;
			std::cout << pAr1.at(iid)->getName() << std::endl;

			GUID elemID = VCDElement_Value;
			tIVCDPropertyElementPtr pFoundElement = p_properties->findElement(guid1, elemID);
			if (pFoundElement != 0)
			{
				std::cout << "Element Found!!!\r\n";
				tIVCDRangePropertyPtr pRange;
				if (pFoundElement->getInterfacePtr(pRange) != 0)
				{
					if (pAr1.at(iid)->getName() == "Focus")
					{
						//std::cout << "FOCUS!!!";
						pRange->setValue(250);
					}

					if (pAr1.at(iid)->getName() == "Exposure")
					{
						//std::cout << "Exp!!!";
						pRange->setValue(-4);
					}
					std::cout << pRange->getValue() << std::endl;
				}

			}

		}

		Grabber::tVidFmtListPtr vformat;
		vformat = grabber1->getAvailableVideoFormats();
		for (Grabber::tVidFmtListPtr::value_type::iterator it = vformat->begin(); it != vformat->end(); it++)
		{
			std::cout << it->c_str() << std::endl;
		}
		grabber1->setVideoFormat("RGB64 (5424x5360)");

		tFrameHandlerSinkPtr pSink = FrameHandlerSink::create(eRGB32, 1);

		pSink->setSnapMode(true);

		grabber1->setSinkType(pSink);

		if (!grabber1->prepareLive(false))
		{
			std::cerr << "Could not render the VideoFormat into a eRGB32 sink.";
			delete grabber1;
			return e;
		}

		// Retrieve the output type and dimension of the handler sink.
		// The dimension of the sink could be different from the VideoFormat, when
		// you use filters.
		FrameTypeInfo info;
		pSink->getOutputFrameType(info);

		BYTE* pBuf[1];
		// Allocate 1 image buffers of the above calculate buffer size.
		for (int i = 0; i < gpowerdata::framebuffersize ; ++i)
		{
			pBuf[i] = new BYTE[info.buffersize];
		}

		// Create a new MemBuffer collection that uses our own image buffers.
		tMemBufferCollectionPtr pCollection = MemBufferCollection::create(info, 1, pBuf);
		if (pCollection == 0 || !pSink->setMemBufferCollection(pCollection))
		{
			std::cerr << "Could not set the new MemBufferCollection, because types do not match.";
			for (int i = 0; i < gpowerdata::framebuffersize; ++i)
			{
				delete pBuf[i];
			}
			delete grabber1;
			return e;
		}



		std::cout << info.dim.cx << " " << info.dim.cy << std::endl;
		std::cout << info.getBitsPerPixel() << std::endl;

		// Start live mode for fast snapping. The live video will not be displayed,
		// because false is passed to startLive().

		grabber1->startLive(false);

		// Snap 5 images. The images are copied to the MemBufferCollection the
		// application created above.
		pSink->snapImages(gpowerdata::framebuffersize);

		// Stop the live video.
		grabber1->stopLive();

		// Close the device.
		grabber1->closeDev();

		cv::Size sz = cv::Size(int(info.dim.cx), int(info.dim.cy));
		int channel = 4;
		IplImage *frame = cvCreateImage(sz, IPL_DEPTH_8U, channel);
		frame->widthStep = int(info.dim.cy);
		frame->imageData = (char *)pBuf[0];
		d.create(int(info.dim.cy), int(info.dim.cx), CV_8UC4);
		d.data = (uchar *)frame->imageData;

		//int step = ((sz.width));
		//cvSetData(frame, (char *)pBuf[0], step);
		//c = cv::cvarrToMat(frame);
		cv::flip(d, e, 0);
		f = e.clone();
		delete grabber1;
		return f;
	}
}

PyObject* maketest(void) {
	cv::Mat s;
	PyObject* pl = PyList_New(3);
	std::string retmessage;
	cv::Mat testimagemat;
	long imsizel = 0;
	testimagemat = inttest();
	if (testimagemat.data == NULL) {
		retmessage = "error make test";
	}
	else {
		retmessage = "Test OK";
		imsizel = testimagemat.total() * testimagemat.elemSize();
	}
	PyList_SetItem(pl, 0, PyUnicode_FromString(retmessage.c_str()));

	PyObject *bar;
	bar = PyByteArray_FromStringAndSize((const char *)testimagemat.data, imsizel);

	PyObject * imsize;
	imsize = PyLong_FromLong(imsizel);

	PyList_SetItem(pl, 1, bar);
	PyList_SetItem(pl, 2, imsize);

	return pl;
}

PyObject* companyname(void) {
	return PyByteArray_FromStringAndSize(a, sizeof(char) * sizeof(a));
}

PyObject* ver(void) {
	return PyLong_FromLong(12);
}

static PyMethodDef powercver[] = {
	{ "p_companyname", (PyCFunction)companyname,METH_NOARGS,nullptr },
	{ "p_maketest", (PyCFunction)maketest,METH_NOARGS,nullptr },
	{ "p_preparecam", (PyCFunction)preparecam,METH_NOARGS,nullptr },
	{ "p_getframe", (PyCFunction)getframe,METH_NOARGS,nullptr },
	{ "p_stoplive", (PyCFunction)stopvideo,METH_NOARGS,nullptr },
	{ "p_startlive", (PyCFunction)startvideo,METH_NOARGS,nullptr },
	{ "p_pushfocus", (PyCFunction)makeautofocus,METH_NOARGS,nullptr },
	{ "p_pushwb", (PyCFunction)makewb,METH_NOARGS,nullptr },
	{ "p_getcaminfo", (PyCFunction)getcaminfo,METH_NOARGS,nullptr },
	{ "p_getfocusval", (PyCFunction)getfocusval,METH_NOARGS,nullptr },
	{ "p_setresolution", (PyCFunction)setresolution, METH_VARARGS ,nullptr },
	{ nullptr, nullptr, 0, nullptr }
};


static PyModuleDef powercmod = {
	PyModuleDef_HEAD_INIT,
	"powerctest",
	"For test only (C++ to Python)",
	0,
	powercver
};

PyMODINIT_FUNC PyInit_powerctest() {
	return PyModule_Create(&powercmod);
}








