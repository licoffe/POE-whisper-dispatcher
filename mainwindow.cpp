#include "mainwindow.h"
#include "ui_mainwindow.h"

/**
 * @brief MainWindow::MainWindow
 *
 * Initialize the main window interface, check if config.cfg file exists and
 * loads it. Updates Token field and Client.txt path accordingly.
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow) {
	ui->setupUi(this);
	this->file_dialog = new QFileDialog( this );
	this->client_path = QString( "" );
	this->stop_parser_thread = false;
	this->statusMessage = new QLabel( "Message" );
	this->last_line_count = -1;
	this->interval = new QTimer( this );
	this->poe_path = "";

	// Read configuration file
	std::string path = QDir::homePath().toStdString() + "/.config/poe-whisper-dispatcher/config.cfg";
	std::ifstream file( path );
	this->ui->log_textedit->append(
				QString( "<span style='text-align: center;'>"
						 "Initialization</span><br>--------------" ));
	if ( file ) {
		this->ui->log_textedit->append( QString( "Found <b>config.cfg</b> file, parsing" ));
		std::regex entry_reg( "(.+) = (.+)" );
		std::smatch match;
		std::string line;
		while ( std::getline( file, line )) {
			qDebug() << QString::fromStdString( line );
			try {
				if ( std::regex_search( line, match, entry_reg ) &&
					 match.size() > 2 ) {
					if ( match.str( 1 ).compare( "token" ) == 0 ) {
						std::string msg = "Found token: <span style='color: rgb(197, 24, 240)'>" + match.str( 2 ) + "</span>";
						this->ui->log_textedit->append( QString::fromStdString( msg ));
						this->ui->pbAccessToken_textfield->setText( QString::fromStdString( match.str( 2 )));
					} else if ( match.str( 1 ).compare( "poe_path" ) == 0 ) {
						std::string msg = "Found PoE path: <span style='color: rgb(197, 24, 240)'>" + match.str( 2 ) + "</span>";
						this->ui->log_textedit->append( QString::fromStdString( msg ));
						std::string poe_path_str = match.str( 2 ) +
												   "Contents/Resources/drive_c/" +
												   "Program Files/Grinding Gear Games/" +
												   "Path Of Exile/logs/Client.txt";
						this->poe_path = QString::fromStdString( poe_path_str );
						this->client_path = QString::fromStdString( match.str(2));
					} else {
						std::string msg = "<span style='color: red'>Found unknown entry (" +
										  match.str( 1 ) + ") while parsing <b>config.cfg</b></span>";
						this->ui->log_textedit->append( QString::fromStdString( msg ));
					}
				}
			} catch ( std::regex_error& e ) {
				qDebug() << "Regex error";
			}
		}
		file.close();
	} else {
		qDebug() << QString::fromStdString( path );
		std::string str = "<span style='color: red'>No " + path + " file found</span>";
		this->ui->log_textedit->append( QString::fromStdString( str ));
		// Check if config folder exists
		std::string config_folder = QDir::homePath().toStdString() +
									"/.config/poe-whisper-dispatcher";
		if ( !QDir( QString::fromStdString( config_folder )).exists()) {
			str = "Config folder does not exist, creating it";
			this->ui->log_textedit->append( QString::fromStdString( str ));
			QDir( QString::fromStdString( config_folder )).mkpath( QString::fromStdString( config_folder ) );
		}
	}
	this->ui->log_textedit->append( QString( "<br>" ));

	QObject::connect( interval, SIGNAL( timeout()), this, SLOT( parse()));
}

MainWindow::~MainWindow() {
	this->stop();
	delete this->statusMessage;
	delete this->file_dialog;
	delete this->interval;
	delete ui;
}

/**
 * @brief MainWindow::browse_to_client_file
 *
 * Opens a dialog for the user to choose which client to parse.
 * @return Path to selected client file
 */
QString MainWindow::browse_to_client_file() {
	QStringList filters;
	filters << "Any files (*)";
	this->file_dialog->setNameFilters( filters );
	this->file_dialog->setFileMode( QFileDialog::Directory );
	this->client_path = this->file_dialog->getExistingDirectory(
							this, tr( "Select folder containing Path Of Exile.app" ),
							"/Applications" );
	qDebug() << "Selected " << this->client_path;
	std::string msg = "Selected <span style='color: rgb(197, 24, 240)'>" +
					  this->client_path.toStdString() + "</span>";
	this->ui->log_textedit->append( QString::fromStdString( msg ));
	std::string full_path = this->client_path.toStdString() +
							"/Path Of Exile.app/Contents/Resources/drive_c/" +
							"Program Files/Grinding Gear Games/" +
							"Path Of Exile/logs/Client.txt";
//	this->client_path = QString::fromStdString( full_path );
	msg = "Attempting to access " + this->client_path.toStdString() + " -> ";
	// Is the provided path accurate
	std::ifstream file( full_path );
	if ( file ) {
		file.close();
		msg += "file exists";
	} else {
		msg += "<span style='color: red'>file does not exist</span>";
		file.close();
	}
	this->ui->log_textedit->append( QString::fromStdString( msg ));
	return this->client_path;
}

/**
 * @brief MainWindow::start
 *
 * Starts the auto-refresh timeout, calling the parse function.
 */
void MainWindow::start() {
	if ( ui->start_button->text().compare( QString( "Start" )) == 0 ) {
		QString token = ui->pbAccessToken_textfield->text();
		// If token is defined
		if ( token.compare( "" ) != 0 ) {
			// Save to config.cfg
			std::string path = QDir::homePath().toStdString() +
							   "/.config/poe-whisper-dispatcher/config.cfg";
			std::ofstream file( path );
			file << "token = " << token.toStdString() << std::endl
				 << "poe_path = " << this->client_path.toStdString() << std::endl;
			file.close();

			this->parse();
			ui->start_button->setText( QString( "Stop" ));
			this->interval->start( this->ui->refreshInterval_slider->value() * 1000 );
			qDebug() << "Starting timer with a frequency of "
					 << this->ui->refreshInterval_slider->value() * 1000
					 << " ms";

			std::ostringstream msg;
			msg << "<span style='color: rgb( 0, 200, 100 )'>Starting dispatcher with a refresh rate of <b>"
				<< this->ui->refreshInterval_slider->value()
				<< "</b> sec</span>";
			this->ui->log_textedit->append( QString::fromStdString( msg.str() ));
		} else {
			this->ui->log_textedit->append( "<span style='color: red'>Error: Invalid token</span>" );
		}
	} else {
		this->stop();
	}
}

/**
 * @brief MainWindow::parse
 */
void MainWindow::parse() {
	this->stop_parser_thread = false;
	// Check if there are some new lines to parse
	std::string full_path = this->client_path.toStdString() +
							"/Path Of Exile.app/Contents/Resources/drive_c/" +
							"Program Files/Grinding Gear Games/" +
							"Path Of Exile/logs/Client.txt";
	int lines = count_lines( full_path );
	if ( lines > this->last_line_count ) {
		qDebug() << "New content, parsing";
		this->parser_thread      = std::thread( &MainWindow::parse_file, this,
												full_path, this->last_line_count );
		this->parser_thread.join();
		this->last_line_count = lines;
	} else {
		qDebug() << "Nothing new";
	}
}

/**
 * @brief MainWindow::stop
 *
 * Stops the auto-refresh timeout. Stops parsing thread if running.
 */
void MainWindow::stop() {
	if ( ui->start_button->text().compare( QString( "Stop" )) == 0 ) {
		qDebug() << "Stoping parsing";
		this->stop_parser_thread = true;
		if ( this->parser_thread.joinable()) {
			this->parser_thread.join();
		}
		ui->start_button->setText( QString( "Start" ));
//		ui->statusBar->showMessage( QString::fromStdString( "Suspended parsing" ));
		qDebug() << "Stopping timer";
		this->ui->log_textedit->append( "<span style='color: rgb( 0, 200, 100 )'>Stopping dispatcher</span>" );
		this->interval->stop();
	}
}
/**
 * @brief MainWindow::parse_file
 *
 * Start parsing client file from input line. If a line matches the whisper
 * regex, dispatch a message through curl using PB's v2 API.
 * @param Path to client file
 * @param Line number from which to start parsing
 */
void MainWindow::parse_file( const std::string file_path, const int start_line ) {
	qDebug() << "Parsing from " << start_line;
	// If file_path is not empty and thread is not already parsing
	if ( file_path.compare( "" ) != 0 ) {
		// If lines have not been counted yet
		if ( this->last_line_count == -1 ) {
			this->last_line_count = count_lines( file_path );
			return;
		}
		qDebug() << "Starting parsing";
		int lines = this->count_lines( file_path );
		std::ostringstream msg;
		msg << "<span style='color: rgb( 240, 175, 24 )'>" << lines - start_line << "</span> lines to parse";
		qDebug() << lines - start_line << " lines to parse";
//		this->ui->log_textedit->append( QString::fromStdString( msg.str() ));
//		this->statusMessage->setText( "Starting parsing" );
//		ui->statusBar->showMessage( QString::fromStdString( "Parsing file " ));
		std::regex whisper_reg( "@([a-zA-Z0-9_]+): ([^\r]*)" );
		std::smatch match;
		// Open file
		std::ifstream file( file_path );
		// Read file line by line
		std::string line;
		int current_line = 1;
		while ( std::getline( file, line ) && !this->stop_parser_thread ) {
//			int progress = ( (double)current_line / (double)lines ) * 100.0;
//			this->ui->progressBar->setValue( progress );
			// We only read starting from start_line index
			if ( current_line > start_line ) {
				try {
					if ( std::regex_search( line, match, whisper_reg ) &&
						 match.size() > 2 ) {
						std::string str =
								"<span style='color: rgb( 240, 164, 24 ); font-weight: bold'>"
								+ match.str(1) + "</span>: " + match.str(2);
						this->ui->log_textedit->append( QString::fromStdString( str ));
						std::ostringstream cmd;
						cmd << "curl -sS --header 'Access-Token: '"
							<< this->ui->pbAccessToken_textfield->text().toStdString()
							<< " --header 'Content-Type: application/json' "
							<< "--data-binary '{\"body\":\"" << match.str(2) << "\","
							<< "\"title\":\"PoE whisper from " << match.str(1)
							<< "\",\"type\":\"note\"}' "
							<< "--request POST https://api.pushbullet.com/v2/pushes&";
						std::string cmd_str = cmd.str();
						std::system( cmd_str.c_str());
					}
				} catch (std::regex_error& e) {
					// Syntax error in the regular expression
					//				qDebug() << e;
				}
			}
			current_line++;
		}
		file.close();
	// Otherwise ...
	} else {
		qDebug() << "Empty file path";
	}
}

/**
 * @brief MainWindow::quit
 *
 * Calls the stop method on quit in order to stop running threads.
 */
void MainWindow::quit() {
	qDebug() << "Exiting";
	this->stop();
}

/**
 * @brief MainWindow::count_lines
 *
 * Count the number of lines in input file.
 * @param Path to text file
 * @return Integer representing the amount of lines
 */
int MainWindow::count_lines( const std::string file_path ) {
	qDebug() << "Checking line count of " << QString::fromStdString( file_path );
	std::ifstream file( file_path );
	  return std::count( std::istreambuf_iterator<char>( file ),
						 std::istreambuf_iterator<char>(), '\n' ) + 1;
}
