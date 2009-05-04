/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "config.h"
#include "version.h"

#include <cassert>

#include "xml/tinyxml.h"

#include <hydrogen/adsr.h>
#include <hydrogen/data_path.h>
#include <hydrogen/LocalFileMng.h>

#include <hydrogen/fx/Effects.h>
#include <hydrogen/globals.h>
#include <hydrogen/Song.h>
#include <hydrogen/sample.h>
#include <hydrogen/instrument.h>
#include <hydrogen/Pattern.h>
#include <hydrogen/note.h>
#include <hydrogen/hydrogen.h>

namespace H2Core
{

Song::Song( const QString& name, const QString& author, float bpm, float volume )
		: Object( "Song" )
		, __is_muted( false )
		, __resolution( 48 )
		, __bpm( bpm )
		, __is_modified( false )
		, __name( name )
		, __author( author )
		, __volume( volume )
		, __metronome_volume( 0.5 )
		, __pattern_list( NULL )
		, __pattern_group_sequence( NULL )
		, __instrument_list( NULL )
		, __filename( "" )
		, __is_loop_enabled( false )
		, __humanize_time_value( 0.0 )
		, __humanize_velocity_value( 0.0 )
		, __swing_factor( 0.0 )
		, __song_mode( PATTERN_MODE )
{
	INFOLOG( QString( "INIT '%1'" ).arg( __name ) );

	//m_bDelayFXEnabled = false;
	//m_fDelayFXWetLevel = 0.8;
	//m_fDelayFXFeedback = 0.5;
	//m_nDelayFXTime = MAX_NOTES / 8;
}



Song::~Song()
{
	// delete all patterns
	delete __pattern_list;

	if ( __pattern_group_sequence ) {
		for ( unsigned i = 0; i < __pattern_group_sequence->size(); ++i ) {
			PatternList *pPatternList = ( *__pattern_group_sequence )[i];
			pPatternList->clear();	// pulisco tutto, i pattern non vanno distrutti qua
			delete pPatternList;
		}
		delete __pattern_group_sequence;
	}

	delete __instrument_list;

	INFOLOG( QString( "DESTROY '%1'" ).arg( __name ) );
}

void Song::purge_instrument( Instrument * I )
{
	for ( int nPattern = 0; nPattern < (int)__pattern_list->get_size(); ++nPattern ) {
		__pattern_list->get( nPattern )->purge_instrument( I );
	}
}


///Load a song from file
Song* Song::load( const QString& filename )
{
	Song *song = NULL;

	SongReader reader;
	song = reader.readSong( filename );

	return song;
}



/// Save a song to file
bool Song::save( const QString& filename )
{
	SongWriter writer;
	int err;
	err = writer.writeSong( this, filename );

	if( err ) {
		return false;
	}
	return QFile::exists( filename );
}


/// Create default song
Song* Song::get_default_song(){
		Song *song = new Song( "empty", "hydrogen", 120, 0.5 );

		song->set_metronome_volume( 0.5 );
		song->set_notes( "..." );
		song->set_license( "" );
		song->set_loop_enabled( false );
		song->set_mode( Song::PATTERN_MODE );
		song->set_humanize_time_value( 0.0 );
		song->set_humanize_velocity_value( 0.0 );
		song->set_swing_factor( 0.0 );

		InstrumentList* pList = new InstrumentList();
		Instrument *pNewInstr = new Instrument(to_string( 0 ), "New instrument", new ADSR());
		pList->add( pNewInstr );
		song->set_instrument_list( pList );
		
		#ifdef JACK_SUPPORT
		Hydrogen::get_instance()->renameJackPorts();
		#endif

		PatternList *patternList = new PatternList();
		Pattern *emptyPattern = Pattern::get_empty_pattern(); 
		emptyPattern->set_name( QString("Pattern 1") ); 
		emptyPattern->set_category( QString("not_categorized") );
		patternList->add( emptyPattern );
		song->set_pattern_list( patternList );
		std::vector<PatternList*>* pPatternGroupVector = new std::vector<PatternList*>;
		PatternList *patternSequence = new PatternList();
		patternSequence->add( emptyPattern );
		pPatternGroupVector->push_back( patternSequence );
		song->set_pattern_group_vector( pPatternGroupVector );
		song->__is_modified = false;
		song->set_filename( "empty_song" );
		
		return song;
}

/// Return an empty song
Song* Song::get_empty_song()
{
	QString dataDir = DataPath::get_data_path();	
	QString filename = dataDir + "/DefaultSong.h2song";

	if( ! QFile::exists( filename ) ){
		_ERRORLOG("File " + filename + " exists not. Failed to load default song.");
		filename = dataDir + "/DefaultSong.h2song";
	}
	
	Song *song = Song::load( filename );
	
	/* if file DefaultSong.h2song not accessible
	 * create a simple default song.
	 */
	if(!song){
		song = Song::get_default_song();
	}

	return song;
}



void Song::set_swing_factor( float factor )
{
	if ( factor < 0.0 ) {
		factor = 0.0;
	} else if ( factor > 1.0 ) {
		factor = 1.0;
	}

	__swing_factor = factor;
}



//::::::::::::::::::::







//-----------------------------------------------------------------------------
//	Implementation of SongReader class
//-----------------------------------------------------------------------------


SongReader::SongReader()
		: Object( "SongReader" )
{
//	infoLog("init");
}



SongReader::~SongReader()
{
//	infoLog("destroy");
}



///
/// Reads a song.
/// return NULL = error reading song file.
///
Song* SongReader::readSong( const QString& filename )
{
	INFOLOG( filename );
	Song* song = NULL;

	if (QFile( filename ).exists() == false ) {
		ERRORLOG( "Song file " + filename + " not found." );
		return NULL;
	}


	#ifdef WIN32
  		TiXmlDocument doc( filename.toAscii().constData() );
	#else
   		TiXmlDocument doc( filename.toUtf8().constData() );
	#endif


	doc.LoadFile();

	TiXmlNode* songNode;	// root element
	if ( !( songNode = doc.FirstChild( "song" ) ) ) {
		ERRORLOG( "Error reading song: song node not found" );
		return NULL;
	}


	m_sSongVersion = LocalFileMng::readXmlString( songNode, "version", "Unknown version" );
	if ( m_sSongVersion != QString( get_version().c_str() ) ) {
		WARNINGLOG( "Trying to load a song created with a different version of hydrogen." );
		WARNINGLOG( "Song [" + filename + "] saved with version " + m_sSongVersion );
	}

	float fBpm = LocalFileMng::readXmlFloat( songNode, "bpm", 120 );
	Hydrogen::get_instance()->setNewBpmJTM( fBpm ); 
	float fVolume = LocalFileMng::readXmlFloat( songNode, "volume", 0.5 );
	float fMetronomeVolume = LocalFileMng::readXmlFloat( songNode, "metronomeVolume", 0.5 );
	QString sName( LocalFileMng::readXmlString( songNode, "name", "Untitled Song" ) );
	QString sAuthor( LocalFileMng::readXmlString( songNode, "author", "Unknown Author" ) );
	QString sNotes( LocalFileMng::readXmlString( songNode, "notes", "..." ) );
	QString sLicense( LocalFileMng::readXmlString( songNode, "license", "Unknown license" ) );
	bool bLoopEnabled = LocalFileMng::readXmlBool( songNode, "loopEnabled", false );

	Song::SongMode nMode = Song::PATTERN_MODE;	// Mode (song/pattern)
	QString sMode = LocalFileMng::readXmlString( songNode, "mode", "pattern" );
	if ( sMode == "song" ) {
		nMode = Song::SONG_MODE;
	}

	float fHumanizeTimeValue = LocalFileMng::readXmlFloat( songNode, "humanize_time", 0.0 );
	float fHumanizeVelocityValue = LocalFileMng::readXmlFloat( songNode, "humanize_velocity", 0.0 );
	float fSwingFactor = LocalFileMng::readXmlFloat( songNode, "swing_factor", 0.0 );

	song = new Song( sName, sAuthor, fBpm, fVolume );
	song->set_metronome_volume( fMetronomeVolume );
	song->set_notes( sNotes );
	song->set_license( sLicense );
	song->set_loop_enabled( bLoopEnabled );
	song->set_mode( nMode );
	song->set_humanize_time_value( fHumanizeTimeValue );
	song->set_humanize_velocity_value( fHumanizeVelocityValue );
	song->set_swing_factor( fSwingFactor );

	/*
	song->m_bDelayFXEnabled = LocalFileMng::readXmlBool( songNode, "delayFXEnabled", false, false );
	song->m_fDelayFXWetLevel = LocalFileMng::readXmlFloat( songNode, "delayFXWetLevel", 1.0, false, false );
	song->m_fDelayFXFeedback= LocalFileMng::readXmlFloat( songNode, "delayFXFeedback", 0.4, false, false );
	song->m_nDelayFXTime = LocalFileMng::readXmlInt( songNode, "delayFXTime", MAX_NOTES / 4, false, false );
	*/


	//  Instrument List
	LocalFileMng localFileMng;
	InstrumentList *instrumentList = new InstrumentList();

	TiXmlNode* instrumentListNode;
	if ( ( instrumentListNode = songNode->FirstChild( "instrumentList" ) ) ) {
		// INSTRUMENT NODE
		int instrumentList_count = 0;
		TiXmlNode* instrumentNode = 0;
		for ( instrumentNode = instrumentListNode->FirstChild( "instrument" ); instrumentNode; instrumentNode = instrumentNode->NextSibling( "instrument" ) ) {
			instrumentList_count++;

			QString sId = LocalFileMng::readXmlString( instrumentNode, "id", "" );			// instrument id
			QString sDrumkit = LocalFileMng::readXmlString( instrumentNode, "drumkit", "" );	// drumkit
			Hydrogen::get_instance()->setCurrentDrumkitname( sDrumkit ); 
			QString sName = LocalFileMng::readXmlString( instrumentNode, "name", "" );		// name
			float fVolume = LocalFileMng::readXmlFloat( instrumentNode, "volume", 1.0 );	// volume
			bool bIsMuted = LocalFileMng::readXmlBool( instrumentNode, "isMuted", false );	// is muted
			float fPan_L = LocalFileMng::readXmlFloat( instrumentNode, "pan_L", 0.5 );	// pan L
			float fPan_R = LocalFileMng::readXmlFloat( instrumentNode, "pan_R", 0.5 );	// pan R
			float fFX1Level = LocalFileMng::readXmlFloat( instrumentNode, "FX1Level", 0.0 );	// FX level
			float fFX2Level = LocalFileMng::readXmlFloat( instrumentNode, "FX2Level", 0.0 );	// FX level
			float fFX3Level = LocalFileMng::readXmlFloat( instrumentNode, "FX3Level", 0.0 );	// FX level
			float fFX4Level = LocalFileMng::readXmlFloat( instrumentNode, "FX4Level", 0.0 );	// FX level
			float fGain = LocalFileMng::readXmlFloat( instrumentNode, "gain", 1.0, false, false );	// instrument gain

			int fAttack = LocalFileMng::readXmlInt( instrumentNode, "Attack", 0, false, false );		// Attack
			int fDecay = LocalFileMng::readXmlInt( instrumentNode, "Decay", 0, false, false );		// Decay
			float fSustain = LocalFileMng::readXmlFloat( instrumentNode, "Sustain", 1.0, false, false );	// Sustain
			int fRelease = LocalFileMng::readXmlInt( instrumentNode, "Release", 1000, false, false );	// Release

			float fRandomPitchFactor = LocalFileMng::readXmlFloat( instrumentNode, "randomPitchFactor", 0.0f, false, false );

			bool bFilterActive = LocalFileMng::readXmlBool( instrumentNode, "filterActive", false );
			float fFilterCutoff = LocalFileMng::readXmlFloat( instrumentNode, "filterCutoff", 1.0f, false );
			float fFilterResonance = LocalFileMng::readXmlFloat( instrumentNode, "filterResonance", 0.0f, false );
			QString sMuteGroup = LocalFileMng::readXmlString( instrumentNode, "muteGroup", "-1", false );
			int nMuteGroup = sMuteGroup.toInt();


			if ( sId == "" ) {
				ERRORLOG( "Empty ID for instrument '" + sName + "'. skipping." );
				continue;
			}


			// create a new instrument
			Instrument *pInstrument = new Instrument( sId, sName, new ADSR( fAttack, fDecay, fSustain, fRelease ) );
			pInstrument->set_volume( fVolume );
			pInstrument->set_muted( bIsMuted );
			pInstrument->set_pan_l( fPan_L );
			pInstrument->set_pan_r( fPan_R );
			pInstrument->set_drumkit_name( sDrumkit );
			pInstrument->set_fx_level( fFX1Level, 0 );
			pInstrument->set_fx_level( fFX2Level, 1 );
			pInstrument->set_fx_level( fFX3Level, 2 );
			pInstrument->set_fx_level( fFX4Level, 3 );
			pInstrument->set_random_pitch_factor( fRandomPitchFactor );
			pInstrument->set_filter_active( bFilterActive );
			pInstrument->set_filter_cutoff( fFilterCutoff );
			pInstrument->set_filter_resonance( fFilterResonance );
			pInstrument->set_gain( fGain );
			pInstrument->set_mute_group( nMuteGroup );

			QString drumkitPath = "";
			if ( ( sDrumkit != "" ) && ( sDrumkit != "-" ) ) {
//				drumkitPath = localFileMng.getDrumkitDirectory( sDrumkit ) + sDrumkit + "/";
				drumkitPath = localFileMng.getDrumkitDirectory( sDrumkit ) + sDrumkit;
			}

			// back compatibility code ( song version <= 0.9.0 )
			TiXmlNode* filenameNode = instrumentNode->FirstChild( "filename" );
			if ( filenameNode ) {
				WARNINGLOG( "Using back compatibility code. filename node found" );
				QString sFilename = LocalFileMng::readXmlString( instrumentNode, "filename", "" );

				if ( drumkitPath != "" ) {
					sFilename = drumkitPath + "/" + sFilename;
				}
				Sample *pSample = Sample::load( sFilename );
				if ( pSample == NULL ) {
					// nel passaggio tra 0.8.2 e 0.9.0 il drumkit di default e' cambiato.
					// Se fallisce provo a caricare il corrispettivo file in formato flac
//					warningLog( "[readSong] Error loading sample: " + sFilename + " not found. Trying to load a flac..." );
					sFilename = sFilename.left( sFilename.length() - 4 );
					sFilename += ".flac";
					pSample = Sample::load( sFilename );
				}
				if ( pSample == NULL ) {
					ERRORLOG( "Error loading sample: " + sFilename + " not found" );
					pInstrument->set_muted( true );
				}
				InstrumentLayer *pLayer = new InstrumentLayer( pSample );
				pInstrument->set_layer( pLayer, 0 );
			}
			//~ back compatibility code
			else {
				unsigned nLayer = 0;
				for ( TiXmlNode* layerNode = instrumentNode->FirstChild( "layer" ); layerNode; layerNode = layerNode->NextSibling( "layer" ) ) {
					if ( nLayer >= MAX_LAYERS ) {
						ERRORLOG( "nLayer > MAX_LAYERS" );
						continue;
					}
					QString sFilename = LocalFileMng::readXmlString( layerNode, "filename", "" );
					float fMin = LocalFileMng::readXmlFloat( layerNode, "min", 0.0 );
					float fMax = LocalFileMng::readXmlFloat( layerNode, "max", 1.0 );
					float fGain = LocalFileMng::readXmlFloat( layerNode, "gain", 1.0 );
					float fPitch = LocalFileMng::readXmlFloat( layerNode, "pitch", 0.0, false, false );

					if ( drumkitPath != "" ) {
						sFilename = drumkitPath + "/" + sFilename;
					}
					Sample *pSample = Sample::load( sFilename );
					if ( pSample == NULL ) {
						ERRORLOG( "Error loading sample: " + sFilename + " not found" );
						pInstrument->set_muted( true );
					}
					InstrumentLayer *pLayer = new InstrumentLayer( pSample );
					pLayer->set_start_velocity( fMin );
					pLayer->set_end_velocity( fMax );
					pLayer->set_gain( fGain );
					pLayer->set_pitch( fPitch );
					pInstrument->set_layer( pLayer, nLayer );
					nLayer++;
				}
			}

			instrumentList->add( pInstrument );
		}
		if ( instrumentList_count == 0 ) {
			WARNINGLOG( "0 instruments?" );
		}

		song->set_instrument_list( instrumentList );
	} else {
		ERRORLOG( "Error reading song: instrumentList node not found" );
		delete song;
		return NULL;
	}





	// Pattern list
	TiXmlNode* patterns = songNode->FirstChild( "patternList" );

	PatternList *patternList = new PatternList();
	int pattern_count = 0;
	TiXmlNode* patternNode = 0;
	for ( patternNode = patterns->FirstChild( "pattern" ); patternNode; patternNode = patternNode->NextSibling( "pattern" ) ) {
		pattern_count++;
		Pattern *pat = getPattern( patternNode, instrumentList );
		if ( pat ) {
			patternList->add( pat );
		} else {
			ERRORLOG( "Error loading pattern" );
			delete patternList;
			delete song;
			return NULL;
		}
	}
	if ( pattern_count == 0 ) {
		WARNINGLOG( "0 patterns?" );
	}
	song->set_pattern_list( patternList );


	// Pattern sequence
	TiXmlNode* patternSequenceNode = songNode->FirstChild( "patternSequence" );

	std::vector<PatternList*>* pPatternGroupVector = new std::vector<PatternList*>;

	// back-compatibility code..
	for ( TiXmlNode* pPatternIDNode = patternSequenceNode->FirstChild( "patternID" ); pPatternIDNode; pPatternIDNode = pPatternIDNode->NextSibling( "patternID" ) ) {
		WARNINGLOG( "Using old patternSequence code for back compatibility" );
		PatternList *patternSequence = new PatternList();
		QString patId = pPatternIDNode->FirstChild()->Value();

		Pattern *pat = NULL;
		for ( unsigned i = 0; i < patternList->get_size(); i++ ) {
			Pattern *tmp = patternList->get( i );
			if ( tmp ) {
				if ( tmp->get_name() == patId ) {
					pat = tmp;
					break;
				}
			}
		}
		if ( pat == NULL ) {
			WARNINGLOG( "patternid not found in patternSequence" );
			continue;
		}
		patternSequence->add( pat );

		pPatternGroupVector->push_back( patternSequence );
	}

	for ( TiXmlNode* groupNode = patternSequenceNode->FirstChild( "group" ); groupNode; groupNode = groupNode->NextSibling( "group" ) ) {
		PatternList *patternSequence = new PatternList();
		for ( TiXmlNode* patternId = groupNode->FirstChild( "patternID" ); patternId; patternId = patternId->NextSibling( "patternID" ) ) {
			QString patId = patternId->FirstChild()->Value();

			Pattern *pat = NULL;
			for ( unsigned i = 0; i < patternList->get_size(); i++ ) {
				Pattern *tmp = patternList->get( i );
				if ( tmp ) {
					if ( tmp->get_name() == patId ) {
						pat = tmp;
						break;
					}
				}
			}
			if ( pat == NULL ) {
				WARNINGLOG( "patternid not found in patternSequence" );
				continue;
			}
			patternSequence->add( pat );
		}
		pPatternGroupVector->push_back( patternSequence );
	}

	song->set_pattern_group_vector( pPatternGroupVector );

#ifdef LADSPA_SUPPORT
	// reset FX
	for ( int fx = 0; fx < MAX_FX; ++fx ) {
		//LadspaFX* pFX = Effects::getInstance()->getLadspaFX( fx );
		//delete pFX;
		Effects::getInstance()->setLadspaFX( NULL, fx );
	}
#endif

	// LADSPA FX
	TiXmlNode* ladspaNode = songNode->FirstChild( "ladspa" );
	if ( ladspaNode ) {
		int nFX = 0;
		TiXmlNode* fxNode;
		for ( fxNode = ladspaNode->FirstChild( "fx" ); fxNode; fxNode = fxNode->NextSibling( "fx" ) ) {
			QString sName = LocalFileMng::readXmlString( fxNode, "name", "" );
			QString sFilename = LocalFileMng::readXmlString( fxNode, "filename", "" );
			bool bEnabled = LocalFileMng::readXmlBool( fxNode, "enabled", false );
			float fVolume = LocalFileMng::readXmlFloat( fxNode, "volume", 1.0 );

			if ( sName != "no plugin" ) {
				// FIXME: il caricamento va fatto fare all'engine, solo lui sa il samplerate esatto
#ifdef LADSPA_SUPPORT
				LadspaFX* pFX = LadspaFX::load( sFilename, sName, 44100 );
				Effects::getInstance()->setLadspaFX( pFX, nFX );
				if ( pFX ) {
					pFX->setEnabled( bEnabled );
					pFX->setVolume( fVolume );
					TiXmlNode* inputControlNode;
					for ( inputControlNode = fxNode->FirstChild( "inputControlPort" ); inputControlNode; inputControlNode = inputControlNode->NextSibling( "inputControlPort" ) ) {
						QString sName = LocalFileMng::readXmlString( inputControlNode, "name", "" );
						float fValue = LocalFileMng::readXmlFloat( inputControlNode, "value", 0.0 );

						for ( unsigned nPort = 0; nPort < pFX->inputControlPorts.size(); nPort++ ) {
							LadspaControlPort *port = pFX->inputControlPorts[ nPort ];
							if ( QString( port->sName ) == sName ) {
								port->fControlValue = fValue;
							}
						}
					}

					TiXmlNode* outputControlNode;
					for ( outputControlNode = fxNode->FirstChild( "outputControlPort" ); outputControlNode; outputControlNode = outputControlNode->NextSibling( "outputControlPort" ) ) {
					}
				}
#endif
			}
			nFX++;
		}
	} else {
		WARNINGLOG( "ladspa node not found" );
	}


	song->__is_modified = false;
	song->set_filename( filename );

	return song;
}



Pattern* SongReader::getPattern( TiXmlNode* pattern, InstrumentList* instrList )
{
	Pattern *pPattern = NULL;

	QString sName = "";	// name
	sName = LocalFileMng::readXmlString( pattern, "name", sName );

	QString sCategory = ""; // category
	sCategory = LocalFileMng::readXmlString( pattern, "category", sCategory );
	int nSize = -1;
	nSize = LocalFileMng::readXmlInt( pattern, "size", nSize, false, false );

	pPattern = new Pattern( sName, sCategory, nSize );



	TiXmlNode* pNoteListNode = pattern->FirstChild( "noteList" );
	if ( pNoteListNode ) {
		// new code :)
		for ( TiXmlNode* noteNode = pNoteListNode->FirstChild( "note" ); noteNode; noteNode = noteNode->NextSibling( "note" ) ) {

			Note* pNote = NULL;

			unsigned nPosition = LocalFileMng::readXmlInt( noteNode, "position", 0 );
			float fLeadLag = LocalFileMng::readXmlFloat( noteNode, "leadlag", 0.0 );
			float fVelocity = LocalFileMng::readXmlFloat( noteNode, "velocity", 0.8f );
			float fPan_L = LocalFileMng::readXmlFloat( noteNode, "pan_L", 0.5 );
			float fPan_R = LocalFileMng::readXmlFloat( noteNode, "pan_R", 0.5 );
			int nLength = LocalFileMng::readXmlInt( noteNode, "length", -1, true );
			float nPitch = LocalFileMng::readXmlFloat( noteNode, "pitch", 0.0, false, false );
			QString sKey = LocalFileMng::readXmlString( noteNode, "key", "C0", false, false );

			QString instrId = LocalFileMng::readXmlString( noteNode, "instrument", "" );

			Instrument *instrRef = NULL;
			// search instrument by ref
			for ( unsigned i = 0; i < instrList->get_size(); i++ ) {
				Instrument *instr = instrList->get( i );
				if ( instrId == instr->get_id() ) {
					instrRef = instr;
					break;
				}
			}
			if ( !instrRef ) {
				ERRORLOG( "Instrument with ID: '" + instrId + "' not found. Note skipped." );
				continue;
			}
			//assert( instrRef );

			pNote = new Note( instrRef, nPosition, fVelocity, fPan_L, fPan_R, nLength, nPitch, Note::stringToKey( sKey ) );
			pNote->set_leadlag(fLeadLag);
			pPattern->note_map.insert( std::make_pair( pNote->get_position(), pNote ) );
		}
	} else {
		// Back compatibility code. Version < 0.9.4
		TiXmlNode* sequenceListNode = pattern->FirstChild( "sequenceList" );

		int sequence_count = 0;
		TiXmlNode* sequenceNode = 0;
		for ( sequenceNode = sequenceListNode->FirstChild( "sequence" ); sequenceNode; sequenceNode = sequenceNode->NextSibling( "sequence" ) ) {
			sequence_count++;

			TiXmlNode* noteListNode = sequenceNode->FirstChild( "noteList" );
			for ( TiXmlNode* noteNode = noteListNode->FirstChild( "note" ); noteNode; noteNode = noteNode->NextSibling( "note" ) ) {

				Note* pNote = NULL;

				unsigned nPosition = LocalFileMng::readXmlInt( noteNode, "position", 0 );
				float fLeadLag = LocalFileMng::readXmlFloat( noteNode, "leadlag", 0.0 , false , false );
				float fVelocity = LocalFileMng::readXmlFloat( noteNode, "velocity", 0.8f );
				float fPan_L = LocalFileMng::readXmlFloat( noteNode, "pan_L", 0.5 );
				float fPan_R = LocalFileMng::readXmlFloat( noteNode, "pan_R", 0.5 );
				int nLength = LocalFileMng::readXmlInt( noteNode, "length", -1, true );
				float nPitch = LocalFileMng::readXmlFloat( noteNode, "pitch", 0.0, false, false );

				QString instrId = LocalFileMng::readXmlString( noteNode, "instrument", "" );

				Instrument *instrRef = NULL;
				// search instrument by ref
				for ( unsigned i = 0; i < instrList->get_size(); i++ ) {
					Instrument *instr = instrList->get( i );
					if ( instrId == instr->get_id() ) {
						instrRef = instr;
						break;
					}
				}
				assert( instrRef );

				pNote = new Note( instrRef, nPosition, fVelocity, fPan_L, fPan_R, nLength, nPitch );
				pNote->set_leadlag(fLeadLag);

				//infoLog( "new note!! pos: " + toString( pNote->m_nPosition ) + "\t instr: " + instrId );
				pPattern->note_map.insert( std::make_pair( pNote->get_position(), pNote ) );
			}
		}
	}

	return pPattern;
}

};




