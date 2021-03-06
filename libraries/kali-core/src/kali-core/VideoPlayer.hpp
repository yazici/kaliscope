/* Copyright (C) 2015 Eloi DU BOIS - All Rights Reserved
 * The license for this file is available here:
 * https://github.com/edubois/kaliscope/blob/master/LICENSE
 */

#ifndef _KALI_CORE_VIDEOCAPTURE_HPP_
#define	_KALI_CORE_VIDEOCAPTURE_HPP_

#include "typedefs.hpp"

#include <mvp-player-core/IVideoPlayer.hpp>
#include <mvp-player-core/IFilePlayer.hpp>
#include <mvp-player-core/Singleton.hpp>

#include <tuttle/common/utils/global.hpp>
#include <tuttle/host/Graph.hpp>
#include <ofxCore.h>
#include <Sequence.hpp>

#include <mutex>
#include <condition_variable>

namespace kaliscope
{

class VideoPlayer : public mvpplayer::IVideoPlayer, public mvpplayer::Singleton<VideoPlayer>
{
public:
    VideoPlayer( const std::shared_ptr<tuttle::host::Graph> & graph = std::shared_ptr<tuttle::host::Graph>() );
    virtual ~VideoPlayer();

// Overrides
public:
    /**
     * @brief reset processing graph to default
     */
    void resetProcessingGraphToDefault();
    
    /**
     * @brief set processing graph
     * @param graph new processing graph
     * @return previous graph
     */
    std::shared_ptr<tuttle::host::Graph> setProcessingGraph( const std::shared_ptr<tuttle::host::Graph> & graph );

    /**
     * @brief get memory cache
     */
    inline tuttle::host::memory::MemoryCache & cache()
    { return _outputCache; }

    /**
     * @brief get time domain definition
     * @return the time domain {min, max}
     */
    OfxRangeD getTimeDomain() const;

    /**
     * @brief get the frames per seconds
     * @return the fps
     */
    double getFPS() const
    {
        assert( _nodeRead != nullptr );
        return 24;  ///<@todo get fps
    }

    /**
     * @brief initialize all
     */
    void initialize();

    /**
     * @brief build processing graph
     */
    void buildGraph();
    
    /**
     * @brief free all
     */
    void terminate();

    /**
     * @brief is the player playing sound
     */
    inline bool isPlaying() const override;

    /**
     * @brief sound volume control for the current played track
     * @param v volume value
     */
    void setVolume( const float v ) override;
    
    /**
     * @brief mute sound
     */
    void mute( const bool active = true ) override;

    /**
     * @brief load a given file
     * @param filename given file
     */
    void load( const boost::filesystem::path & filename ) override;

    /**
     * @brief frees the sound object
     */
    void unload() override;

    /**
     * @brief plays a sound
     * @param pause pause playing
     * @return false on success, true if error
     */
    bool play (const bool pause = false) override;

    /**
     * @brief get a frame at a certain time
     * @param nFrame frame number in time domain
     * @return an image, null of error
     */
    virtual DefaultImageT getFrame( const double nFrame );

    /**
     * @brief get current frame at a certain time
     * @return an image, null of error
     */
    DefaultImageT getFrame()
    { return getFrame( _currentPosition ); }

    /**
     * @brief set current track position
     * @param[in] position position in percent (0-100), ms or frames
     * @param[in] seekType seek position in frame, percent or milliseconds
     * @return true on success, false if error
     */
    bool setPosition( const double position, const mvpplayer::ESeekPosition seekType = mvpplayer::eSeekPositionSample ) override;

    /**
     * @brief get the current track's position
     * @return the current position in milliseconds
     */
    std::size_t getPosition() const override;

    /**
     * @brief get the current track's length
     * @return the current length in milliseconds
     */
    std::size_t getLength() const override;

    /**
     * @brief restart track
     */
    bool restart() override;

    /**
     * @brief checks whether the sound is on or off
     */
    bool getSound();

    /**
     * @brief pause or unpause the current track
     * @param pause boolean setting the pause
     */
    void setPause( const bool pause ) override;

    /**
     * @brief switch sound off/on
     * @param sound sound on/off
     */
    void setSound( const bool sound ) override;

    /**
     * @brief toggle sound on or off
     */
    void toggleSound() override;

    /**
     * @brief toggle pause on or off
     */
    void togglePause() override;

    /**
     * @brief is track paused
     */
    bool isPaused() const override;

    /**
     * @brief set output filename
     * @param nFrame[in] frame number
     * @param nbTotalFrames[in] total frame number
     * @param filePathPrefix[in] file path prefix
     * @param extenstion[in] file extension
     * @warning if the final node is not a writer, this will have no effect
     */
    void setOutputFilename( const double nFrame, const std::size_t nbTotalFrames, const std::string & filePathPrefix, const std::string & extension );

    /**
     * @brief set output filename
     * @param filePath[in] output file path
     */
    void setOutputFilename( const std::string & filePath );

    /**
     * @brief set output filename
     * @param filePath[in] input file path
     * @param isSequence[in] is filepath a sequence
     */
    void setInputFilename( const boost::filesystem::path & filePath, const bool isSequence = false );

    /**
     * @brief initialize sequence
     * @param filePath[in] full file path
     */
    void initSequence( const std::string & filePath );

    /**
     * @brief get frame step
     * @return a double for the frame step (usually 1)
     */
    double getFrameStep() const
    { return _frameStep; }

// Various
private:
    std::unique_ptr<sequenceParser::Sequence> _inputSequence;   ///< Used to play sequence of images
    double _frameStep = 1.0;            ///< Frame stepping (default: one frame)
    double _currentPosition = 0.0;      ///< Current track position
    double _currentLength = 0.0;        ///< Current track length
    double _currentFPS = 0.0;           ///< Current frames per seconds
    bool _playing = false;              ///< 'Is playing track' status

// Thread related
private:
    std::mutex _mutexPlayer;                              ///< Mutex thread

// TuttleOFX related
private:
    tuttle::host::Graph::Node *_nodeFinal = nullptr;        ///< Final effect node
    tuttle::host::Graph::Node *_nodeRead = nullptr;         ///< File reader
    tuttle::host::Graph::Node *_nodeWrite = nullptr;        ///< File wirter
    tuttle::host::memory::MemoryCache _outputCache;         ///< Cache for video output
    std::shared_ptr<tuttle::host::Graph> _graph;                ///< effects processing graph
};

}

#endif
