/*
 * viewerexample.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: Chris Denham
 */

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#include "openvrviewer.h"
#include "openvreventhandler.h"

class GraphicsWindowViewer : public osgViewer::Viewer
{
public:
	GraphicsWindowViewer(osg::ArgumentParser& arguments, osgViewer::GraphicsWindow* graphicsWindow)
		: osgViewer::Viewer(arguments), _graphicsWindow(graphicsWindow) { }

	virtual void eventTraversal()
	{
		if (_graphicsWindow.valid() && _graphicsWindow->checkEvents())
		{
			osgGA::EventQueue::Events events;
			_graphicsWindow->getEventQueue()->copyEvents(events);
			osgGA::EventQueue::Events::iterator itr;
			for (itr = events.begin(); itr != events.end(); ++itr)
			{
				osgGA::GUIEventAdapter* event = dynamic_cast<osgGA::GUIEventAdapter*>(itr->get());
				if (event != nullptr && event->getEventType() == osgGA::GUIEventAdapter::CLOSE_WINDOW)
				{
					// We have "peeked" at the event queue for the GraphicsWindow and 
					// found a CLOSE_WINDOW event. This would normally mean that OSG 
					// is about to release OpenGL contexts attached to the window.
					// For some applications it is better to make the application
					// terminate in a more "normal" way e.g. as it does when <Esc>
					// key had been pressed.
					// In this way we can safely perform any cleanup required after
					// termination of the Viewer::run() loop, i.e. cleanup that would 
					// otherwise malfunction if the earlier processing of the CLOSE_WINDOW
					// event had already released required OpenGL contexts.
					// So, here we "get in early" and remove the close event and append
					// a quit application event.
					events.erase(itr);
					_graphicsWindow->getEventQueue()->setEvents(events);
					_graphicsWindow->getEventQueue()->quitApplication();
					break;
				}
			}
		}
		osgViewer::Viewer::eventTraversal();
	}
private:
	osg::ref_ptr<osgViewer::GraphicsWindow> _graphicsWindow;
};

int main( int argc, char** argv )
{
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc, argv);
	// read the scene from the list of file specified command line arguments.
	osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

	// if not loaded assume no arguments passed in, try use default cow model instead.
	if (!loadedModel) { loadedModel = osgDB::readNodeFile("cow.osgt"); }

	// Still no loaded model, then exit
	if (!loadedModel)
	{
		osg::notify(osg::ALWAYS) << "No model could be loaded and didn't find cow.osgt, terminating.." << std::endl;
		return 0;
	}

	// Create Trackball manipulator
	osg::ref_ptr<osgGA::CameraManipulator> cameraManipulator = new osgGA::TrackballManipulator;
	const osg::BoundingSphere& bs = loadedModel->getBound();

	if (bs.valid())
	{
		// Adjust view to object view
      /* This caused a problem with the head tracking on the Vive
		osg::Vec3 modelCenter = bs.center();
		osg::Vec3 eyePos = bs.center() + osg::Vec3(0, bs.radius(), 0);
		cameraManipulator->setHomePosition(eyePos, modelCenter, osg::Vec3(0, 0, 1));
      */
	}

	// Exit if we do not have an HMD present
	if (!OpenVRDevice::hmdPresent())
	{
		osg::notify(osg::FATAL) << "Error: No valid HMD present!" << std::endl;
		return 1;
	}

	// Open the HMD
	float nearClip = 0.01f;
	float farClip = 10000.0f;
	float worldUnitsPerMetre = 1.0f;
	int samples = 4;
	osg::ref_ptr<OpenVRDevice> openvrDevice = new OpenVRDevice(nearClip, farClip, worldUnitsPerMetre, samples);

	// Exit if we fail to initialize the HMD device
	if (!openvrDevice->hmdInitialized())
	{
		// The reason for failure was reported by the constructor.
		return 1;
	}

	// Get the suggested context traits
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = openvrDevice->graphicsContextTraits();
	traits->windowName = "OsgOpenVRViewerExample";

	// Create a graphic context based on our desired traits
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits);

	if (!gc)
	{
		osg::notify(osg::NOTICE) << "Error, GraphicsWindow has not been created successfully" << std::endl;
		return 1;
	}

	if (gc.valid())
	{
		gc->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));
		gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	GraphicsWindowViewer viewer(arguments, dynamic_cast<osgViewer::GraphicsWindow*>(gc.get()));

	// Force single threaded to make sure that no other thread can use the GL context
	viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
	viewer.getCamera()->setGraphicsContext(gc);
	viewer.getCamera()->setViewport(0, 0, traits->width, traits->height);

	// Disable automatic computation of near and far plane
	viewer.getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	viewer.setCameraManipulator(cameraManipulator);

	// Things to do when viewer is realized
	osg::ref_ptr<OpenVRRealizeOperation> openvrRealizeOperation = new OpenVRRealizeOperation(openvrDevice);
	viewer.setRealizeOperation(openvrRealizeOperation.get());

	osg::ref_ptr<OpenVRViewer> openvrViewer = new OpenVRViewer(&viewer, openvrDevice, openvrRealizeOperation);
	openvrViewer->addChild(loadedModel);
	viewer.setSceneData(openvrViewer);
	// Add statistics handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	viewer.addEventHandler(new OpenVREventHandler(openvrDevice));

	viewer.run();

	// Need to do this here to make it happen before destruction of the OSG Viewer, which destroys the OpenGL context.
	openvrDevice->shutdown(gc.get());

	return 0;
}
