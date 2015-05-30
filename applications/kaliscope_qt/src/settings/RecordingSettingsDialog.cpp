#include "RecordingSettingsDialog.hpp"
#include "PluginListDialog.hpp"
#include "EditPluginParamsDialog.hpp"

#include <kali-core/settingsTools.hpp>

#include <boost-adds/environment.hpp>

#include <QtCore/QSize>
#include <QtGui/QDropEvent>
#include <QtCore/QDir>
#include <QtWidgets/QFileDialog>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>

namespace kaliscope
{
namespace gui
{
namespace qt
{

RecordingSettingsDialog::RecordingSettingsDialog( QWidget *parent )
: QDialog( parent )
{
    widget.setupUi( this );
    connect( widget.btnAddPlugin, SIGNAL( released() ), this, SLOT( addPlugin() ) );
    connect( widget.btnRemovePlugin, SIGNAL( released() ), this, SLOT( removePluginSelection() ) );
    connect( widget.btnLoadConfig, SIGNAL( released() ), this, SLOT( loadConfig() ) );
    connect( widget.btnSaveConfig, SIGNAL( released() ), this, SLOT( saveConfig() ) );
    connect( widget.comboPresets, SIGNAL( currentIndexChanged( const int ) ), this, SLOT( loadPreset( const int ) ) );

    widget.listPipeline->setMovement( QListView::Static );
    widget.listPipeline->setDragDropMode( QAbstractItemView::InternalMove );
    connect( widget.listPipeline, SIGNAL( itemDoubleClicked(QListWidgetItem *) ), this, SLOT( editPluginParams( QListWidgetItem * ) ) );
    widget.listPipeline->viewport()->installEventFilter( this );

    _pipelineSettings.read( QDir::homePath().toStdString() + "/" + kKaliscopeDefaultPipelineSettingsFilename );
    buildPipelineFrom( _pipelineSettings );
    loadPresetItems();
}

RecordingSettingsDialog::~RecordingSettingsDialog()
{
}

void RecordingSettingsDialog::loadPresetItems()
{
    using namespace boost::filesystem;
    // Try to read from application path
    path presetsPath( current_path() / kPresetsDirectory );
    directory_iterator it( presetsPath ), eod;

    widget.comboPresets->clear();
    _presets.clear();
    int i = 0;
    BOOST_FOREACH( path const &p, std::make_pair(it, eod) )   
    {
        if( is_regular_file( p ) )
        {
            if ( _presets[i].read( p ) )
            {
                QString itemText = QString::fromStdString( _presets[i].get<std::string>( "", "presetName" ) );
                if ( itemText.isEmpty() )
                {
                    _presets[i].set( "", "presetName", p.filename().string() );
                    itemText = QString::fromStdString( p.filename().string() );
                }
                widget.comboPresets->addItem( itemText, QVariant( QString::fromStdString( p.string() ) ) );
                ++i;
            }
        } 
    }
}

void RecordingSettingsDialog::loadPreset( const int index )
{
    auto itSettings = _presets.find( index );
    if ( itSettings != _presets.end() )
    {
        _pipelineSettings = itSettings->second;
        buildPipelineFrom( _pipelineSettings );
    }
}

void RecordingSettingsDialog::loadConfig()
{
    static QString sFilePath;
    if ( !sFilePath.size() )
    {
        sFilePath = QString::fromStdString( ( boost::filesystem::current_path() / kPresetsDirectory ).string() );
    }

    sFilePath = QFileDialog::getOpenFileName( this, tr("Pipeline configuration file"), sFilePath, tr("Config files (*.json *.xml *.ini)") );
    if ( sFilePath.size() )
    {
        boost::filesystem::path filepath( sFilePath.toStdString() );
        _pipelineSettings.read( filepath );
        buildPipelineFrom( _pipelineSettings );

        QString presetName = QString::fromStdString( _pipelineSettings.get<std::string>( "", "presetName" ) );
        
        if ( !presetName.isEmpty() )
        {
            const int index = widget.comboPresets->findText( presetName );
            if ( index == -1 )
            {
                widget.comboPresets->addItem( presetName, QVariant( sFilePath ) );
                widget.comboPresets->setCurrentIndex( index );
            }
            else
            {
                widget.comboPresets->setEditText( presetName );
            }
        }
        else
        {
            _pipelineSettings.set( "", "presetName", filepath.filename().string() );
            widget.comboPresets->setEditText( QString::fromStdString( filepath.filename().string() ) );
        }
    }
}

void RecordingSettingsDialog::saveConfig()
{
    static QString sFilePath;
    if ( !sFilePath.size() )
    {
        sFilePath = QString::fromStdString( ( boost::filesystem::current_path() / kPresetsDirectory ).string() );
    }

    const int index = widget.comboPresets->findText( widget.comboPresets->lineEdit()->text() );
    if ( index != -1 )
    {
        sFilePath = widget.comboPresets->itemData( index ).toString();
    }
    else
    {
        sFilePath = QFileDialog::getSaveFileName( this, tr("Pipeline configuration file"), sFilePath, tr("Config files (*.json *.xml *.ini)") );
    }

    if ( sFilePath.size() )
    {
        _pipelineSettings.set( "", "presetName", widget.comboPresets->lineEdit()->text().toStdString() );
        _pipelineSettings.write( sFilePath.toStdString() );
    }
}

bool RecordingSettingsDialog::eventFilter( QObject* sender, QEvent* event )
{
    if ( event->type() == QEvent::Drop )
    {
        const bool res = sender->eventFilter( sender, event );
        if ( res )
        {
            rebuildPipelineSettings();
        }
        return res;
    }
    return false;
}

void RecordingSettingsDialog::buildPipelineFrom( const mvpplayer::Settings & pipelineSettings )
{
    widget.listPipeline->clear();
    std::map<PluginItem, mvpplayer::Settings> pluginsSettings = splitOfxNodesSettings( _pipelineSettings );
    for( const auto &p: pluginsSettings )
    {
        using namespace tuttle::host;
        try
        {
            ofx::imageEffect::OfxhImageEffectPlugin* plugFx = core().getImageEffectPluginById( p.first.pluginIdentifier );
            if ( !plugFx )
            {
                continue;
            }
            plugFx->loadAndDescribeActions();

            addPlugin( *plugFx, p.second );
        }
        catch( ... )
        {
            std::cerr << "Unable to load plugin: " << p.first.pluginIdentifier << std::endl;
        }
    }
}

void RecordingSettingsDialog::rebuildPipelineSettings()
{
    const int cnt = widget.listPipeline->count();
    _pipelineSettings.clear();
    for( int i = 0; i < cnt; ++i )
    {
        TablePluginItem * pluginItem = static_cast<TablePluginItem*>( widget.listPipeline->item( i ) );
        _pipelineSettings.set( std::string(), std::to_string( i ), pluginItem->settings().tree() );
    }
}

void RecordingSettingsDialog::removePluginSelection()
{
    if ( widget.listPipeline->currentItem() )
    {
        qDeleteAll( widget.listPipeline->selectedItems() );
        rebuildPipelineSettings();
    }
}

void RecordingSettingsDialog::addPlugin()
{
    PluginListDialog dlg( !widget.listPipeline->count(), this );
    if ( dlg.exec() && dlg.selectedPlugin() )
    {
        mvpplayer::Settings nodeSettings;
        TablePluginItem *plugItem = addPlugin( *dlg.selectedPlugin(), nodeSettings );

        // Edit plugin parameters
        editPluginParams( plugItem );
    }
}

TablePluginItem * RecordingSettingsDialog::addPlugin( const tuttle::host::ofx::imageEffect::OfxhImageEffectPlugin & plugin, const mvpplayer::Settings & settings )
{
    TablePluginItem *plugItem = new TablePluginItem( plugin, settings );
    widget.listPipeline->addItem( plugItem );
    QWidget *w = buildPluginWidgetFrom( plugItem );
    widget.listPipeline->setItemWidget( plugItem, w );
    plugItem->setSizeHint( w->sizeHint() );
    widget.listPipeline->setGridSize( QSize( w->sizeHint().width(), 0 ) );
    return plugItem;
}

QWidget* RecordingSettingsDialog::buildPluginWidgetFrom( TablePluginItem *plugItem )
{
    QWidget *w = new QWidget( widget.listPipeline );
    QVBoxLayout *layout = new QVBoxLayout();
    w->setLayout( layout );
    w->setContentsMargins( 0, 0, 0, 0 );

    const tuttle::host::ofx::imageEffect::OfxhImageEffectPlugin & plugin = plugItem->plugin();
    const std::string itemString = ( boost::format( "%1% - v%2%.%3%" ) % plugin.getIdentifier() % plugin.getVersionMajor() % plugin.getVersionMinor() ).str();

    QIcon iconStd;
    iconStd.addFile( QStringLiteral(":/kaliscope/icons/plugins/puzzle.png"), QSize( 100, 100 ) );
    QLabel *icon = new QLabel();
    icon->setContentsMargins( 0, 0, 0, 0 );
    icon->setPixmap( iconStd.pixmap( 64, 64 ) );
    layout->addWidget( icon );
    icon->setToolTip( QString::fromStdString( itemString ) );

    QLabel *caption = new QLabel( QString::fromStdString( itemString ) );
    caption->setContentsMargins( 0, 0, 0, 0 );
    caption->setFixedWidth( 128 );
    caption->setWordWrap( true );
    layout->addWidget( caption );

    return w;
}

void RecordingSettingsDialog::editPluginParams( QListWidgetItem * item )
{
    try
    {
        TablePluginItem * pluginItem = static_cast<TablePluginItem*>( item );
        const int itemIndex = widget.listPipeline->row( item );

        PluginItem key( itemIndex, pluginItem->plugin().getIdentifier() );
        mvpplayer::Settings & currentSettings = pluginItem->settings();

        std::unique_ptr<tuttle::host::INode> plugNode( tuttle::host::createNode( pluginItem->plugin().getIdentifier() ) );
        EditPluginParamsDialog dlg( pluginItem->plugin().getIdentifier(), *plugNode, &currentSettings, this );
        if ( dlg.exec() )
        {
            currentSettings = dlg.nodeSettings();
        }
    }
    catch( ... )
    {
        TUTTLE_LOG_CURRENT_EXCEPTION;
    }
}

void RecordingSettingsDialog::accept()
{
    rebuildPipelineSettings();
    Parent::accept();
}

}
}
}
