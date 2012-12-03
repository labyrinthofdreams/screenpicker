#include <ffms.h>
#ifdef _WIN32
#include <objbase.h>
#endif
#include "videoframegrabber.h"

vfg::VideoFrameGrabber::VideoFrameGrabber(QObject *parent) :
    QObject(parent),
    com_inited(false),
    videoSource(NULL),
    numFrames(0),
    currentFrame(0)
{
    // Initialize COM on Windows
    // This is required for FFMS
#ifdef _WIN32
    HRESULT res = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(res))
        com_inited = true;
    else if (res != RPC_E_CHANGED_MODE) {
        /* com initialization failed, handle error */
    }
#endif

    // Initialize FFMS
    FFMS_Init(0, 0);
    // Set up FFMS error reporting
    errorInfo.Buffer = errorBuffer;
    errorInfo.BufferSize = sizeof(errorBuffer);
    errorInfo.ErrorType = FFMS_ERROR_SUCCESS;
    errorInfo.SubType = FFMS_ERROR_SUCCESS;
    // Set pixel formats
    pixelFormats[0] = FFMS_GetPixFmt("rgb24");
    pixelFormats[1] = -1;
}

vfg::VideoFrameGrabber::~VideoFrameGrabber()
{
#ifdef _WIN32
    if (com_inited)
        CoUninitialize();
#endif

    if(videoSource != NULL)
    {
        FFMS_DestroyVideoSource(videoSource);
    }
}

void vfg::VideoFrameGrabber::load(QString filename)
{
    // Create index
    FFMS_Index *index = FFMS_MakeIndex(filename.toStdString().c_str(), 0, 0, NULL, NULL, FFMS_IEH_ABORT, NULL, NULL, &errorInfo);
    if(index == NULL)
    {
        emit errorOccurred(tr("Failed to create index"));
        return;
    }

    // Retrieve first video track
    int trackNo = FFMS_GetFirstTrackOfType(index, FFMS_TYPE_VIDEO, &errorInfo);
    if(trackNo < 0)
    {
        emit errorOccurred(tr("Failed to retrieve video track from the file."));
        return;
    }

    // Create video source
    if(videoSource != NULL)
    {
        FFMS_DestroyVideoSource(videoSource);
    }

    videoSource = FFMS_CreateVideoSource(filename.toStdString().c_str(), trackNo, index, 1, FFMS_SEEK_NORMAL, &errorInfo);
    if(videoSource == NULL)
    {
        emit errorOccurred(tr("Failed to create video source object."));
        return;
    }

    // Destroy index as it's unneeded
    FFMS_DestroyIndex(index);

    const FFMS_Frame *propFrame = FFMS_GetFrame(videoSource, 0, &errorInfo);

    if(FFMS_SetOutputFormatV2(videoSource, pixelFormats, propFrame->EncodedWidth, propFrame->EncodedHeight, FFMS_RESIZER_BICUBIC, &errorInfo))
    {
        emit errorOccurred(tr("Failed to set output format for the video."));
        return;
    }

    const FFMS_VideoProperties *videoProps = FFMS_GetVideoProperties(videoSource);
    currentFrame = 0;
    numFrames = videoProps->NumFrames - 1;
    emit videoReady(videoProps);
}

bool vfg::VideoFrameGrabber::hasVideo() const
{
    return (videoSource != NULL);
}

unsigned vfg::VideoFrameGrabber::lastFrame() const
{
    return currentFrame + 1;
}

const FFMS_Frame* vfg::VideoFrameGrabber::internalGetFrame(unsigned frameNum)
{
    if(frameNum > numFrames)
    {
        emit errorOccurred(tr("Frame number over range: %1").arg(frameNum));
        return NULL;
    }

    const FFMS_Frame *curFrame = FFMS_GetFrame(videoSource, frameNum, &errorInfo);
    if (curFrame == NULL)
    {
        emit errorOccurred(tr("Failed to retrieve frame: %1").arg(frameNum));
        return NULL;
    }

    currentFrame = frameNum;
    return curFrame;
}

void vfg::VideoFrameGrabber::requestFrame(unsigned frameNum)
{
    // Because frame requests are between range 1 - n
    --frameNum;

    const FFMS_Frame* frame = internalGetFrame(frameNum);

    if(frame != NULL)
        emit frameGrabbed(frame);
}

void vfg::VideoFrameGrabber::requestNextFrame()
{
    if(currentFrame == numFrames)
    {
        emit errorOccurred(tr("Reached last frame"));
        return;
    }

    ++currentFrame;
    const FFMS_Frame* frame = internalGetFrame(currentFrame);

    if(frame != NULL)
        emit frameGrabbed(frame);
}

void vfg::VideoFrameGrabber::requestPreviousFrame()
{
    if(currentFrame == 0)
    {
        emit errorOccurred(tr("Reached first frame"));
        return;
    }

    --currentFrame;
    const FFMS_Frame* frame = internalGetFrame(currentFrame);

    if(frame != NULL)
        emit frameGrabbed(frame);
}

const FFMS_Frame* vfg::VideoFrameGrabber::getFrame(unsigned frameNum)
{
    --frameNum;

    const FFMS_Frame* frame = internalGetFrame(frameNum);
    return frame;
}

unsigned vfg::VideoFrameGrabber::totalFrames() const
{
    return numFrames + 1;
}

QImage vfg::convertToQImage(const FFMS_Frame *frame)
{
    if(frame == NULL)
        return QImage();

    // Scanline consists of each RGB channel, so one line is
    // three frame widths since there's Red, Green and Blue channels, each separated
    const unsigned scanlineLen = frame->ScaledWidth * 3;
    // Create QImage from image data
    QImage img(frame->ScaledWidth, frame->ScaledHeight, QImage::Format_RGB888);
    for(unsigned y = 0; y < frame->ScaledHeight; ++y)
    {
        memcpy(img.scanLine(y), frame->Data[0] + y * frame->Linesize[0], scanlineLen);
    }

    return img;
}
