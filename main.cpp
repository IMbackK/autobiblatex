#include <iostream>
#include <filesystem>
#include <memory>
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-global.h>
#include <sstream>
#include <curl/curl.h>
#include <cassert>
#include <fstream>
#include <ctime>

#include "log.h"
#include "tokenize.h"


constexpr char CROSSREF_URL_A[] = "https://api.crossref.org/works/";
constexpr char CROSSREF_URL_B[] = "/transform/application/x-bibtex";

static void printUsage(int argc, char** argv)
{
	Log(Log::INFO)<<"An application that, given a directory, will attempt to create a biblatex bibliography of the pdfs contained within";
	Log(Log::INFO)<<"Usage: "<<argv[0]<<" [FOLDER] [OUTPUT FILENAME]";
}

static void dropMessage(const std::string& message, void* userdata)
{
	(void)message;
	(void)userdata;
}

static std::string getText(poppler::document* document)
{
	std::string text;
	size_t pageCount = document->pages();
	for(size_t i = 0; i < pageCount; ++i)
		text.append(document->create_page(i)->text().to_latin1());
	return text;
}

static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

static std::string wgetUrl(std::string url)
{
	CURL* curlContext = curl_easy_init();
	if(!curlContext)
	{
		Log(Log::ERROR)<<"Could not init curl";
		return "";
	}

	Log(Log::INFO)<<"Trying "<<url;
	char errorBuffer[CURL_ERROR_SIZE] = "";
	std::string buffer;
	CURLcode ret;

	ret = curl_easy_setopt(curlContext, CURLOPT_ERRORBUFFER, errorBuffer);
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_URL, url.c_str());
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_WRITEFUNCTION, writeCallback);
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_WRITEDATA, &buffer);
	assert(ret == CURLE_OK);
	ret = curl_easy_setopt(curlContext, CURLOPT_SERVER_RESPONSE_TIMEOUT, 10);
	ret = curl_easy_perform(curlContext);
	if(ret != CURLE_OK)
	{
		Log(Log::WARN)<<"Could not load from "<<url<<" curl retuned errno "<<ret<<" message: "<<errorBuffer;
		return "";
	}
	curl_easy_cleanup(curlContext);

	return buffer;
}

static std::string bibtexEntryFromCrossref(poppler::document* document)
{
	std::string documentText = getText(document);
	std::stringstream ss(documentText);
	if(ss.seekg(ss.str().find("doi.org"), std::ios_base::beg))
	{
		std::string doi;
		std::getline(ss, doi, '\n');
		doi = tokenize(doi, ' ')[0];
		doi.erase(0, 8);
		Log(Log::INFO)<<"Found doi "<<doi;
		std::string crossrefUrl(CROSSREF_URL_A);
		crossrefUrl.append(doi);
		crossrefUrl.append(CROSSREF_URL_B);
		std::string bibtexEntry = wgetUrl(crossrefUrl);
		if(bibtexEntry.empty() || bibtexEntry.find("not found") != std::string::npos)
		{
			Log(Log::INFO)<<"could not get bibtex entry from crossref";
			return "";
		}
		else
		{
			return bibtexEntry;
		}
	}

	Log(Log::INFO)<<"Document dose not contain doi";
	return "";
}

static std::string bibtexEntryFromHeuristic(poppler::document* document, std::string basename)
{
	std::string keywords = document->get_keywords().to_latin1();
	std::string title = document->get_title().to_latin1();
	std::string author = document->get_author().to_latin1();
	std::string subject = document->get_subject().to_latin1();
	std::string creator = document->get_creator().to_latin1();
	time_t cretionTime = document->get_creation_date_t();
	if(title.empty() || author.empty())
	{
		Log(Log::WARN)<<"Coult not create bibtex entry by heuristic";
		return "";
	}

	struct tm* timeS = gmtime(&cretionTime);

	std::string bibtexEntry;
	bibtexEntry.append("@article{");
	bibtexEntry.append(basename);
	bibtexEntry.append(",\n");
	bibtexEntry.append("\tyear = ");
	bibtexEntry.append(std::to_string(timeS->tm_year+1900));
	if(!creator.empty())
	{
		bibtexEntry.append(",\n");
		bibtexEntry.append("\tpublisher = {");
		bibtexEntry.append(creator);
		bibtexEntry.push_back('}');
	}
	bibtexEntry.append(",\n");
	bibtexEntry.append("\tauthor = {");
	bibtexEntry.append(author);
	bibtexEntry.append("},\n");
	bibtexEntry.append("\ttitle = {");
	bibtexEntry.append(title);
	bibtexEntry.push_back('}');
	if(!subject.empty())
	{
		bibtexEntry.append(",\n");
		bibtexEntry.append("\tjournal = {");
		bibtexEntry.append(subject);
		bibtexEntry.push_back('}');
	}
	bibtexEntry.append("\n}");
	return bibtexEntry;
}

int main(int argc, char** argv)
{
	Log::level = Log::DEBUG;
	poppler::set_debug_error_function(dropMessage, nullptr);

	if(argc < 3)
	{
		printUsage(argc, argv);
		return 0;
	}
	else if(argc > 3)
	{
		Log(Log::WARN)<<"This application will process only one directory at a time. Will process: "<<argv[1];
	}

	std::fstream file;
	file.open(argv[2], std::ios_base::out);
	if(!file.is_open())
	{
		Log(Log::ERROR)<<"Could not open "<<argv[2]<<" for writeing";
		return 2;
	}

	std::fstream rejectFile;
	rejectFile.open("rej.txt", std::ios_base::out);
	if(!rejectFile.is_open())
		Log(Log::WARN)<<"Could not open reject file at rej.txt";

	std::filesystem::path dirPath(argv[1]);
	if(std::filesystem::is_directory(dirPath))
	{
		for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dirPath))
		{
			if((entry.is_regular_file() || entry.is_symlink()) && entry.path().extension() == std::string(".pdf"))
			{
				Log(Log::DEBUG)<<"Processing "<<entry.path();
				poppler::document* document = poppler::document::load_from_file(entry.path());
				if(!document)
				{
					Log(Log::WARN)<<"Could not load "<<entry.path();
					continue;
				}

				std::string bibtexEntry = bibtexEntryFromCrossref(document);
				if(!bibtexEntry.empty())
				{
					file<<bibtexEntry<<'\n'<<'\n';
					continue;
				}

				bibtexEntry = bibtexEntryFromHeuristic(document, entry.path().stem());
				if(!bibtexEntry.empty())
				{
					file<<bibtexEntry<<'\n'<<'\n';
				}
				else
				{
					Log(Log::WARN)<<"Could not create entry for "<<entry.path();
					if(rejectFile.is_open())
						rejectFile<<entry.path()<<'\n';
				}
			}
		}
	}
	else
	{
		Log(Log::ERROR)<<dirPath<<" must be a valid directory";
		return 1;
	}

	rejectFile.close();
	file.close();

	return 0;
}
