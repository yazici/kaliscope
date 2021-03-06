/* Copyright (C) 2015 Eloi DU BOIS - All Rights Reserved
 * The license for this file is available here:
 * https://github.com/edubois/kaliscope/blob/master/LICENSE
 */

#ifndef _KALISCOPEENGINE_HPP_
#define	_KALISCOPEENGINE_HPP_

#include "typedefs.hpp"
#include "VideoPlayer.hpp"

#include <mvp-player-core/MVPPlayerEngine.hpp>

#include <boost-adds/thread/Semaphore.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/typedefs.hpp>
#include <boost/thread.hpp>

#include <thread>

namespace kaliscope
{

/**
 * @brief kaliscope engine
 */
class KaliscopeEngine : public mvpplayer::MVPPlayerEngine
{
private:
    typedef mvpplayer::MVPPlayerEngine Parent;
    typedef KaliscopeEngine This;
public:
    KaliscopeEngine( VideoPlayer *videoPlayer );
    virtual ~KaliscopeEngine();

    /**
     * @brief terminate video player
     */
    inline void terminate()
    { _videoPlayer->terminate(); }

    /**
     * @brief set processing graph
     * @param graph new processing graph
     * @return previous processing graph
     */
    std::shared_ptr<tuttle::host::Graph> setProcessingGraph( const std::shared_ptr<tuttle::host::Graph> & graph );

    /**
     * @brief start processing thread
     */
    void start();

    /**
     * @brief stop playing
     */
    void stop();

    /**
     * @brief play a given file
     * @return false on success, true if error
     */
    bool playFile( const boost::filesystem::path & filename ) override;
    
    /**
     * @brief tells the engine that a has been processed,
     *         and that we can process the next one
     * @param nFrame frame number
     */
    void frameProcessed( const double nFrame );

    /**
     * @brief use frame stepping or not
     * @param active active or not
     */
    void setFrameStepping( const bool active = true )
    { _frameStepping = active; }

    /**
     * @brief process next frame
     */
    inline void processNextFrame()
    { _semaphoreFrameStepping.post(); }

    const boost::filesystem::path & inputFilePath() const
    { return _inputFilePath; }

    void setInputFilePath( const boost::filesystem::path & path )
    { _inputFilePath = path; }

    const std::string & outputFilePathPrefix() const
    { return _outputFilePathPrefix; }

    const std::string & outputFileExtension() const
    { return _outputFileExtension; }

    void setOutputFilePathPrefix( const std::string & prefix )
    { _outputFilePathPrefix = prefix; }

    void setOutputFileExtension( const std::string & extension )
    { _outputFileExtension = extension; }

    void setIsInputSequence( const bool isSequence )
    { _isInputSequence = isSequence; }

    void setIsOutputSequence( const bool isSequence )
    { _isOutputSequence = isSequence; }

private:

    /**
     * @brief stop worker thread
     */
    void stopWorker();

    /**
     * @brief used to read video step by step
     */
    void playWork();

// Signals
public:
    boost::signals2::signal<void( const std::size_t nFrame, const DefaultImageT image )> signalFrameReady;   ///< Signals that a new frame is ready

// Various
private:
    VideoPlayer *_videoPlayer = nullptr;                ///< Pointer to the video player
    bool _stopped = false;
    bool _frameStepping = false;                        ///< Frame stepping
    boost::filesystem::path _inputFilePath;             ///< Input path
    std::string _outputFilePathPrefix;                  ///< Output path prefix
    std::string _outputFileExtension;                   ///< Output file extension
    bool _isInputSequence = false;                      ///< Is input a sequence ?
    bool _isOutputSequence = false;                     ///< Is output a sequence ?

// Thread related
private:
    std::mutex _mutexPlayer;                            ///< Mutex thread
    std::mutex _mutexSynchro;                           ///< Mutex thread
    boost::Semaphore _semaphoreSynchro;                 ///< Synchronization semaphore
    boost::Semaphore _semaphoreFrameStepping;           ///< To play step by step
    std::unique_ptr<std::thread> _playerThread;         ///< Player's thread
};

}

#endif
