
#ifndef H2C_FILESYSTEM_H
#define H2C_FILESYSTEM_H

#include <hydrogen/object.h>
#include <QtCore/QString>

namespace H2Core
{

/**
 * Filesystem is a thin layer over QDir, QFile and QFileInfo
 */
class Filesystem : public H2Core::Object
{
		H2_OBJECT
	public:
		/** flags available for Filesystem::check_permissions */
		enum file_perms {
			is_dir =0x01,
			is_file=0x02,
			is_readable=0x04,
			is_writable=0x08,
			is_executable=0x10
		};
		static const QString songs_ext;
		static const QString scripts_ext;
		static const QString patterns_ext;
		static const QString playlist_ext;
		static const QString songs_filter_name;
		static const QString scripts_filter_name;
		static const QString patterns_filter_name;
		static const QString playlists_filter_name;

		/**
		 * check user and system filesystem usability
		 * \param logger is a pointer to the logger instance which will be used
		 * \param sys_path an alternate system data path
		 */
		static bool bootstrap( Logger* logger, const QString& sys_path=0 );

		/** returns system data path */
		static QString sys_data_path();
		/** returns user data path */
		static QString usr_data_path();

		/** returns user ladspa paths */
		static QStringList ladspa_paths();

		/** returns system config path */
		static QString sys_config_path();
		/** returns user config path */
		static QString usr_config_path();
		/** returns system empty sample file path */
		static QString empty_sample_path();
		/** returns system empty song file path */
		static QString empty_song_path();
		/** returns untitled song file name */
		static QString untitled_song_file_name();
		/** returns system click file path */
		static QString click_file_path();
		/** returns click file path from user directory if exists, otherwise from system */
		static QString usr_click_file_path();
		/** returns the path to the drumkit XSD (xml schema definition) file */
		static QString drumkit_xsd_path( );
		/** returns the path to the pattern XSD (xml schema definition) file */
		static QString pattern_xsd_path( );
		/** returns the path to the playlist pattern XSD (xml schema definition) file */
		static QString playlist_xsd_path( );

		/** returns gui image path */
		static QString img_dir();
		/** returns documentation path */
		static QString doc_dir();
		/** returns internationalization path */
		static QString i18n_dir();
		/** returns user scripts path */
		static QString scripts_dir();
		/** returns user songs path */
		static QString songs_dir();
		/** returns user song path, add file extension */
		static QString song_path( const QString& sg_name );
		/** returns user patterns path */
		static QString patterns_dir();
		/** returns user patterns path for a specific drumkit */
		static QString patterns_dir( const QString& dk_name );
		/** returns user patterns path, add file extension*/
		static QString pattern_path( const QString& dk_name, const QString& p_name );
		/** returns user plugins path */
		static QString plugins_dir();
		/** returns system drumkits path */
		static QString sys_drumkits_dir();
		/** returns user drumkits path */
		static QString usr_drumkits_dir();
		/** returns user playlist path */
		static QString playlists_dir();
		/** returns user playlist path, add file extension */
		static QString playlist_path( const QString& pl_name );
		/** returns untitled playlist file name */
		static QString untitled_playlist_file_name();
		/** returns user cache path */
		static QString cache_dir();
		/** returns user repository cache path */
		static QString repositories_cache_dir();
		/** returns system demos path */
		static QString demos_dir();
		/** returns system xsd path */
		static QString xsd_dir();
		/** returns temp path */
		static QString tmp_dir();
		/**
		 * touch a temporary file under tmp_dir() and return it's path.
		 * if base has a suffix it will be preserved, spaces will be replaced by underscores.
		 * \param base, part of the path
		 */
		static QString tmp_file_path( const QString& base );

		/* DRUMKIT */
		/** Checks if a given file is part of a kit in the soundlibrary or external */
		static bool file_is_partof_drumkit( const QString& fname);
		/** returns list of usable system drumkits ( see Filesystem::drumkit_list ) */
		static QStringList sys_drumkit_list( );
		/** returns list of usable user drumkits ( see Filesystem::drumkit_list ) */
		static QStringList usr_drumkit_list( );
		/**
		 * returns true if the drumkit exists within usable system or user drumkits
		 * \param dk_name the drumkit name
		 */
		static bool drumkit_exists( const QString& dk_name );
		/**
		 * returns path for a drumkit within user drumkit path
		 * \param dk_name the drumkit name
		 */
		static QString drumkit_usr_path( const QString& dk_name );
		/**
		 * returns path for a drumkit searching within user then system drumkits
		 * \param dk_name the drumkit name
		 */
		static QString drumkit_path_search( const QString& dk_name );
		/**
		 * returns the directory holding the named drumkit searching within user then system drumkits
		 * \param dk_name the drumkit name
		 */
		static QString drumkit_dir_search( const QString& dk_name );
		/**
		 * returns true if the path contains a usable drumkit
		 * \param dk_location the root drumkit location
		 */
		static bool drumkit_valid( const QString& dk_path );
		/**
		 * returns the path to the xml file within a suposed drumkit path
		 * \param dk_path the path to the drumkit
		 */
		static QString drumkit_file( const QString& dk_path );

		/* PATTERNS */
		/**
		 * returns a list of existing drumkit sub dir into the patterns directory
		 */
		static QStringList pattern_drumkits();
		/**
		 * returns a list of existing patterns
		 */
		static QStringList pattern_list();
		/**
		 * returns a list of existing patterns
		 * \param p_path the path to look for patterns in
		 */
		static QStringList pattern_list( const QString& path );

		/* SONGS */
		/** returns a list of existing songs */
		static QStringList song_list( );
		/** returns a list of existing songs, excluding the autosaved one */
		static QStringList song_list_cleared( );
		/**
		 * returns true if the song file exists
		 * \param sg_name the song name
		 */
		static bool song_exists( const QString& sg_name );

		/** send current settings information to logger with INFO severity */
		static void info();

		/* PLAYLISTS */
		/** returns a list of existing playlists */
		static QStringList playlist_list( );

		/**
		 * returns true if the given path is an existing regular file
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool file_exists( const QString& path, bool silent=false );
		/**
		 * returns true if the given path is an existing readable regular file
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool file_readable( const QString& path, bool silent=false );
		/**
		 * returns true if the given path is a possibly writable file (may exist or not)
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool file_writable( const QString& path, bool silent=false );
		/**
		 * returns true if the given path is an existing executable regular file
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool file_executable( const QString& path, bool silent=false );
		/**
		 * returns true if the given path is a readable regular directory
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool dir_readable( const QString& path, bool silent=false );
		/**
		 * returns true if the given path is a writable regular directory
		 * \param path the path to the file to check
		 * \param silent output not messages if set to true
		 */
		static bool dir_writable( const QString& path, bool silent=false );
		/**
		 * returns true if the path is a readable and writable regular directory, create if it not exists
		 * \param path the path to the file to check
		 * \param create will try to create path if not exists and set to true
		 * \param silent output not messages if set to true
		 */
		static bool path_usable( const QString& path, bool create=true, bool silent=false );
		/**
		 * writes to a file
		 * \param dst the destination path
		 * \param content then string to write
		 */
		static bool write_to_file( const QString& dst, const QString& content );
		/**
		 * copy a source file to a destination
		 * \param src source file path
		 * \param dsr destination file path
		 * \param overwrite allow to overwrite an existing file if set to true
		 */
		static bool file_copy( const QString& src, const QString& dst, bool overwrite=false );
		/**
		 * remove a path
		 * \param path the path to be removed
		 * \param recursive performe recursive removal if set to true
		 */
		static bool rm( const QString& path, bool recursive=false );
		/**
		 * create a path
		 * \param path the path to the directory to be created
		 */
		static bool mkdir( const QString& path );

	private:
		static Logger* __logger;                    ///< a pointer to the logger
		static bool check_sys_paths();              ///< returns true if the system path is consistent
		static bool check_usr_paths();              ///< returns true if the user path is consistent
		static bool rm_fr( const QString& path );   ///< recursively remove a path

		/**
		 * returns a list of usable drumkits, which means having a readable drumkit.xml file
		 * \param path the path to search in for drumkits
		 */
		static QStringList drumkit_list( const QString& path );
		/**
		 * return true if all the asked permissions are ok
		 * \param path the path to the file to check
		 * \param perms bit mask of file_perms
		 * \param silent output not messages if set to true
		 */
		static bool check_permissions( const QString& path, const int perms, bool silent );

		static QString __sys_data_path;     ///< the path to the system files
		static QString __usr_data_path;     ///< the path to the user files
		static QString __usr_cfg_path;      ///< the path to the user config file
		static QStringList __ladspa_paths;  ///< paths to laspa plugins
};

};

#endif  // H2C_FILESYSTEM_H

/* vim: set softtabstop=4 noexpandtab: */
