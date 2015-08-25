#ifndef _TUTTLE_PLUGIN_FILTERCLEANER_DEFINITIONS_HPP_
#define _TUTTLE_PLUGIN_FILTERCLEANER_DEFINITIONS_HPP_

#include <tuttle/plugin/global.hpp>

namespace tuttle {
namespace plugin {
namespace colorMaskRemover {

enum EParamAlgo
{
    eParamAlgoRGBReduction,
    eParamAlgoYUVReduction
};

static const std::string kParamHelpButton( "Help" );
static const std::string kParamHelpLabel( "Help" );
static const std::string kParamHelpString( "Remove orange filter that appears on film scans.\nYou must first analyze an image containing only the filter (or put the RGB value of the filter)." );

static const std::string kParamAnalyzeButton( "Analyse orange mask" );
static const std::string kParamAnalyzeLabel( "Analyse orange mask" );
static const std::string kParamApplyParameters( "Apply" );
static const std::string kParamAnalyzeHint( "Use this button on an image supposed to be white to get the right mask color (you need to click twice: one to get the color, one to apply)" );

static const std::string kParamFilterForceNewRender( "Force new render" );

static const std::string kParamMaximumValue( "Maximum channel value" );
static const std::string kParamMaximumValueLabel( "Maximum channel value" );
static const int kParamDefaultMaximumValue( 255 );

static const std::string kParamRedFilterColor( "Red filter color" );
static const std::string kParamRedFilterColorLabel( "Red filter color" );
static const unsigned short kParamDefaultRedFilterColor( 247 );

static const std::string kParamGreenFilterColor( "Green filter color" );
static const std::string kParamGreenFilterColorLabel( "Green filter color" );
static const unsigned short kParamDefaultGreenFilterColor( 133 );

static const std::string kParamBlueFilterColor( "Blue filter color" );
static const std::string kParamBlueFilterColorLabel( "Blue filter color" );
static const unsigned short kParamDefaultBlueFilterColor( 78 );

static const std::string kParamRedFactor( "Red channel factor (%)" );
static const std::string kParamRedFactorLabel( "Red channel factor (%)" );
static const double kParamDefaultRedFactorColor( 100.0 );

static const std::string kParamGreenFactor( "Green channel factor (%)" );
static const std::string kParamGreenFactorLabel( "Green channel factor (%)" );
static const double kParamDefaultGreenFactorColor( 100.0 );

static const std::string kParamBlueFactor( "Blue channel factor (%)" );
static const std::string kParamBlueFactorLabel( "Blue channel factor (%)" );
static const double kParamDefaultBlueFactorColor( 100.0 );

static const std::string kParamAlgorithm( "Algorithm" );
static const std::string kParamAlgorithmRGBReduction( "RGB reduction" );
static const std::string kParamAlgorithmYUVReduction( "YUV reduction" );

static const std::string kParamColorInvert( "Invert colors" );
static const std::string kParamColorInvertLabel( "Invert colors" );
static const bool kParamDefaultColorInvertValue( false );

}
}
}

#endif
