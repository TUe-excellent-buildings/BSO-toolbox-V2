#ifndef BSO_DATA_PLOT_CPP
#define BSO_DATA_PLOT_CPP

#include <fstream>
#include <string>
#include <sstream>

namespace bso { namespace utilities {

data_plot::data_plot()
{

} // ctor()

data_plot::data_plot(const std::vector<data_point>& data, std::vector<std::string> axisDataTypes, std::string plotName)
:   axisDataTypes(axisDataTypes), dataName(plotName)
{
	for (const auto& i : data)
	{
		mData.push_back(std::make_shared<data_point>(i));
	}
} // ctor()

data_plot::data_plot(utilities::data_plot& rhs)
{
	mData = rhs.mData;
    axisDataTypes = rhs.axisDataTypes;
	dataName = rhs.dataName;
	scatterData = rhs.scatterData;
	paretoData = rhs.paretoData;
	vectorStyleData = rhs.vectorStyleData;
} // ctor()

data_plot::~data_plot()
{
	
} // dtor()

void data_plot::writeDataToFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName, std::string dataKind /*= ""*/)
{ 
    if(fileName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(data.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected some data to be stored within the input variable data\n"
										 << "The found size of data is: " << data.size() << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    
    // Output mData to fileName in the format gnuPlot can read later on.
    std::ofstream output;
	if (!fileName.empty()) output.open(fileName.c_str());
	
	if (fileName.empty() || !output.is_open())
	{
		std::stringstream errorMessage;
		errorMessage << "Could not open the following file to write a dataFile for plotting to:" << std::endl
								 << ((fileName.empty())? "no input file given" : fileName) << std::endl
								 << "(bso/utilities/data_plot.cpp). " << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	
    output << "# ";
    for(std::string i: axisDataTypes)
    {
        output << i << "\t";
    }
    output << std::endl;
    for(const auto& i: data)
    {
        for(int j=0; j< (*i).size(); j++)
        {     
            output << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*i)(j) << "\t";
        }
        output << std::endl;
    }

    if(dataKind == "scatterData") {scatterData = fileName;}
    if(dataKind == "paretoData") {paretoData = fileName;}
    if(dataKind == "nonParetoData") {nonParetoData = fileName;}
} // writeDataToFile()

void data_plot::writeVectorStyleFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName)
{
    if(fileName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(data.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected some data to be stored within the input variable data\n"
										 << "The found size of data is: " << data.size() << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Output mData to fileName in the format gnuPlot can read later on.
    std::ofstream output;
	if (!fileName.empty()) output.open(fileName.c_str());
	
	if (fileName.empty() || !output.is_open())
	{
		std::stringstream errorMessage;
		errorMessage << "Could not open the following file to write a dataFile for plotting to:" << std::endl
								 << ((fileName.empty())? "no input file given" : fileName) << std::endl
								 << "(bso/utilities/data_plot.cpp). " << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	
    output << "# ";
    for(std::string i: axisDataTypes)
    {
        output << i << "\t";
    }
    for(std::string i: axisDataTypes)
    {
        output << i << "2\t";
    }
    output << std::endl;

    for (auto it = data.begin(); it != data.end(); it++) 
    {
        const auto& i = *it;
        if (std::next(it) != data.end()) 
        {
            const auto& next_i = *std::next(it);

            for(int j=0; j< (*i).size(); j++)
            {
                output  << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*i)(j) << "\t";
            }
            for(int j=0; j< (*next_i).size(); j++)
            {
                output << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << ((*next_i)(j)-(*i)(j)) << "\t";
            }
            output << std::endl;
        }
    }

    // Store the just generated file in object
    vectorStyleData = fileName;
} // writeVectorStyleFile()

void data_plot::addToCombinedDataFile(std::string fileName, std::string runKind, std::string designName)
{ 
    if(fileName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(designName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a designName with at least one character, but the designName name did not had any.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(runKind == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a runKind with at least one character, but the runKind name did not had any.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(mData.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected some data to be stored within the input variable data\n"
										 << "The found size of data is: " << mData.size() << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Check ouput file:
    std::ifstream inputFile(fileName);
    std::ofstream outputFile("temp.txt");
    if (!outputFile.is_open())
    { 
        std::stringstream errorMessage;
				errorMessage << "\nError, unable to open file.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Get relevant data to write to file:
    // Make a normalized copy for space iterations:
    std::vector<utilities::data_point> mDataNormalized;
    for(const auto i: mData)
    {
        mDataNormalized.push_back(*i);
    }
    utilities::data_point utopianPoint;
	utopianPoint.setToUtopia(mDataNormalized);
    utilities::data_point distopianPoint;
	distopianPoint.setToDistopia(mDataNormalized);
    std::swap(utopianPoint(1),distopianPoint(1));
    for (auto& j : mDataNormalized)
    {
        j.normalize(utopianPoint,distopianPoint,{1});
    }
    utopianPoint.setToUtopia(mDataNormalized);
	distopianPoint.setToDistopia(mDataNormalized);

    // Determine the sd Best, bp Best and kneePoint design
    int kneePointIteration = 0;
    int sdBestIteration = 0;
    int bpBestIteration = 0;
    utilities::data_point sdBest = *(mData[0]);
    utilities::data_point sdBestNormalized = mDataNormalized[0];
    utilities::data_point bpBest = *(mData[0]);
    utilities::data_point bpBestNormalized = mDataNormalized[0];
    utilities::data_point kneePoint;
    utilities::data_point kneePointObjective({0,1});
    std::vector<data_point> kneePointVectorNor = kneePointObjective.findClosestIn(mDataNormalized);
    for(int i=0; i < mData.size(); i++)
    {
        if(mDataNormalized[i](0) < sdBestNormalized(0))
        { // determine the best design in SD
            sdBest = *(mData[i]);
            sdBestNormalized = mDataNormalized[i];
            sdBestIteration = i;
        }
        if(mDataNormalized[i](1) > bpBestNormalized(1))
        { // determine the best design in BP
            bpBest = *(mData[i]);
            bpBestNormalized = mDataNormalized[i];
            bpBestIteration = i;
        }
        if(kneePointVectorNor[0] == mDataNormalized[i])
        {
            kneePoint = *(mData[i]);
            kneePointIteration = i;
        }
    }

    double SDBestImprovementPercentage = (1-((*(mData[sdBestIteration]))(0)/(*(mData[0]))(0)))*100;
    double BPBestImprovementPercentage = (1-((*(mData[bpBestIteration]))(1)/(*(mData[0]))(1)))*100;
    double SDLastImprovementPercentage = (1-((*(mData[mData.size()-1]))(0)/(*(mData[0]))(0)))*100;
    double BPLastImprovementPercentage = (1-((*(mData[mData.size()-1]))(1)/(*(mData[0]))(1)))*100;
    
    if (!inputFile.is_open())
    {
        // File does not exist yet, so initialize with headers and first data values
        outputFile << "," << designName << runKind << "," << designName << runKind << "," << std::endl;
        outputFile << ",";
        for(std::string i: axisDataTypes)
        {
            outputFile << i << ",";
        }
        outputFile << std::endl;
        int iteration = 0;
        
        for(const auto& i: mData)
        {
            outputFile << iteration << ",";
            for(int j=0; j< (*i).size(); j++)
            {     
                outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*i)(j) << ",";
            }
            outputFile << std::endl;
            iteration++;
        }
        outputFile << "kneePoint,";
        for(int j=0; j< kneePoint.size(); j++){outputFile << kneePoint(j) << ",";} 
        outputFile << std::endl;
        outputFile << "sdBest,";
        for(int j=0; j< sdBest.size(); j++){outputFile << sdBest(j) << ",";} 
        outputFile << std::endl;
        outputFile << "bpBest,";
        for(int j=0; j< bpBest.size(); j++){outputFile << bpBest(j) << ",";} 
        outputFile << std::endl;
        outputFile << "kneePoint iteration," << kneePointIteration << ",," << std::endl;
        outputFile << "sdBest iteration," << sdBestIteration << ",," << std::endl;
        outputFile << "bpBest iteration," << bpBestIteration << ",," << std::endl;
        outputFile << "SD last iteration vs. first iteration," << SDLastImprovementPercentage << ",," << std::endl;
        outputFile << "BP last iteration vs. first iteration," << BPLastImprovementPercentage << ",," << std::endl;
        outputFile << "SD best iteration vs. first iteration," << SDBestImprovementPercentage << ",," << std::endl;
        outputFile << "BP best iteration vs. first iteration," << BPBestImprovementPercentage << ",," << std::endl;

        inputFile.close();
        outputFile.close();
    } 
    else 
    {
        // File is not empty, process existing lines
        int lineCount = 0;
        int dataCount = 0;
        std::string line;
        while (getline(inputFile, line)) 
        {
            if(lineCount == 0){line += designName + runKind + "," + designName + runKind + ",";}
            else if(lineCount == 1)
            {
                for(std::string i: axisDataTypes)
                {
                    line +=  i  + ",";
                }
            }
            else if(dataCount < mData.size())
            {
                for(int j=0; j< (*(mData[dataCount])).size(); j++)
                {     
                    // Use ostringstream to format the double with full precision
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*(mData[dataCount]))(j) << ",";
                    // Append the formatted value to the line
                    line += oss.str();
                }
                dataCount++;
            }
            else
            {
                std::ostringstream oss;
                if(mData.size()+2 == lineCount)
                {
                    for(int j=0; j< kneePoint.size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << kneePoint(j) << ",";} 
                }
                else if(mData.size()+3 == lineCount)
                {
                    for(int j=0; j< sdBest.size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBest(j) << ",";} 
                }
                else if(mData.size()+4 == lineCount)
                {
                    for(int j=0; j< bpBest.size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBest(j) << ",";} 
                }
                else if(mData.size()+5 == lineCount)
                {
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << kneePointIteration << ",,";
                }
                else if(mData.size()+6 == lineCount)
                {
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBestIteration << ",,";
                }
                else if(mData.size()+7 == lineCount)
                {
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBestIteration << ",,";
                }
                else if(mData.size()+8 == lineCount)
                {
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << SDLastImprovementPercentage << ",,";
                }
                else if(mData.size()+9 == lineCount)
                {
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << BPLastImprovementPercentage << ",,";
                }
                else if(mData.size()+10 == lineCount)
                {
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << SDBestImprovementPercentage << ",,";
                }
                else if(mData.size()+11 == lineCount)
                {
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << BPBestImprovementPercentage << ",,";
                }
                line += oss.str();
            }
            // Write the modified line to the temporary file
            outputFile << line << std::endl;
            lineCount++;
        }

        inputFile.close();
        outputFile.close();

        // Remove original file
        if (std::remove(fileName.c_str()) != 0) 
        {
            std::stringstream errorMessage;
                errorMessage << "\nError, while deleting original file.\n"
                                        << "(bso/utilities/data_plot.cpp). " << std::endl;
                throw std::invalid_argument(errorMessage.str());
        } 
    }
    

    // Rename the temporary file to the original file name
    if (std::rename("temp.txt", fileName.c_str()) != 0) 
    {
        std::stringstream errorMessage;
            errorMessage << "\nError, renaming temporary file.\n"
                                    << "(bso/utilities/data_plot.cpp). " << std::endl;
            throw std::invalid_argument(errorMessage.str());
    } 
    else 
    {            
        std::cout << "File processing complete and original file overwritten." << std::endl;
    }
    
} // addToCombinedDataFile()

void data_plot::addToOverviewCombinedDataFile(std::string fileName, std::string runKind, std::string designName)
{ 
    if(fileName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(designName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a designName with at least one character, but the designName name did not had any.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(runKind == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a runKind with at least one character, but the runKind name did not had any.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(mData.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected some data to be stored within the input variable data\n"
										 << "The found size of data is: " << mData.size() << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Check ouput file:
    std::ifstream inputFile(fileName);
    std::ofstream outputFile("temp.txt");
    if (!outputFile.is_open())
    { 
        std::stringstream errorMessage;
				errorMessage << "\nError, unable to open file.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Get relevant data to write to file:
    // Make a normalized copy for space iterations:
    std::vector<utilities::data_point> mDataNormalized;
    for(const auto i: mData)
    {
        mDataNormalized.push_back(*i);
    }
    utilities::data_point utopianPoint;
	utopianPoint.setToUtopia(mDataNormalized);
    utilities::data_point distopianPoint;
	distopianPoint.setToDistopia(mDataNormalized);
    std::swap(utopianPoint(1),distopianPoint(1));
    for (auto& j : mDataNormalized)
    {
        j.normalize(utopianPoint,distopianPoint,{1});
    }
    utopianPoint.setToUtopia(mDataNormalized);
	distopianPoint.setToDistopia(mDataNormalized);

    // Determine the sd Best, bp Best and kneePoint design
    int kneePointIteration = 0;
    int sdBestIteration = 0;
    int bpBestIteration = 0;
    utilities::data_point sdBest = *(mData[0]);
    utilities::data_point sdBestNormalized = mDataNormalized[0];
    utilities::data_point bpBest = *(mData[0]);
    utilities::data_point bpBestNormalized = mDataNormalized[0];
    utilities::data_point kneePoint;
    utilities::data_point kneePointObjective({0,1});
    std::vector<data_point> kneePointVectorNor = kneePointObjective.findClosestIn(mDataNormalized);
    for(int i=0; i < mData.size(); i++)
    {
        if(mDataNormalized[i](0) < sdBestNormalized(0))
        { // determine the best design in SD
            sdBest = *(mData[i]);
            sdBestNormalized = mDataNormalized[i];
            sdBestIteration = i;
        }
        if(mDataNormalized[i](1) > bpBestNormalized(1))
        { // determine the best design in BP
            bpBest = *(mData[i]);
            bpBestNormalized = mDataNormalized[i];
            bpBestIteration = i;
        }
        if(kneePointVectorNor[0] == mDataNormalized[i])
        {
            kneePoint = *(mData[i]);
            kneePointIteration = i;
        }
    }

    double SDBestImprovementPercentage = (1-((*(mData[sdBestIteration]))(0)/(*(mData[0]))(0)))*100;
    double BPBestImprovementPercentage = (1-((*(mData[bpBestIteration]))(1)/(*(mData[0]))(1)))*100;
    double SDLastImprovementPercentage = (1-((*(mData[mData.size()-1]))(0)/(*(mData[0]))(0)))*100;
    double BPLastImprovementPercentage = (1-((*(mData[mData.size()-1]))(1)/(*(mData[0]))(1)))*100;
    
    if (!inputFile.is_open())
    {
        // initiate descriptive line:
        outputFile << ",best sd design in sd run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "best bp design in sd run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "Last design of sd run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        for(int j=0; j < 6; j++) outputFile << ",";

        outputFile << "best bp design in bp run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "best sd design in bp run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "Last design of bp run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        for(int j=0; j < 6; j++) outputFile << ",";

        outputFile << "best sd design in weighted run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "best bp design in weighted run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "Last design of weighted run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << std::endl;
        
        // make rule with value indicators:
        outputFile << ",";
        for(int j=0; j < 3; j++)
        {
            for(std::string i: axisDataTypes)
            {
                outputFile <<  i  + ",";
            }
        }
        outputFile << "sd best iteration,bp best iteration,sd improvement last vs. initial,bp improvement last vs. initial,sd improvement best vs. initial,bp improvement best vs. initial,";
        for(int j=0; j < 3; j++)
        {
            for(std::string i: axisDataTypes)
            {
                outputFile <<  i  + ",";
            }
        }
        outputFile << "sd best iteration,bp best iteration,sd improvement last vs. initial,bp improvement last vs. initial,sd improvement best vs. initial,bp improvement best vs. initial,";
        for(int j=0; j < 3; j++)
        {
            for(std::string i: axisDataTypes)
            {
                outputFile <<  i  + ",";
            }
        }
        outputFile << "sd best iteration,bp best iteration,sd improvement last vs. initial,bp improvement last vs. initial,sd improvement best vs. initial,bp improvement best vs. initial,";
        outputFile << std::endl;

        // output performance rule of data:
        int lastIndex =  mData.size()-1;
        outputFile << designName << ",";
        for(int j=0; j < sdBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBest(j) << ",";} 
        for(int j=0; j < bpBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBest(j) << ",";} 
        for(int j=0; j < (*(mData[lastIndex])).size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*(mData[lastIndex]))(j) << ",";} // performance of last design of run
        outputFile << sdBestIteration << "," ;
        outputFile << bpBestIteration << "," ;
        outputFile << SDLastImprovementPercentage << "," ;
        outputFile << BPLastImprovementPercentage << "," ;
        outputFile << SDBestImprovementPercentage << "," ;
        outputFile << BPBestImprovementPercentage << "," ;

        inputFile.close();
        outputFile.close();
    } 
    else 
    {
        // Is there already a line for the input design
        int lineCount = 0;
        std::string line;
        bool lineFound = false;
        while (getline(inputFile, line)) 
        {
            if(lineCount == 0 || lineCount == 1)
            {
                outputFile << line << std::endl;
                lineCount++; 
                continue;
            }
            if(designName == line.substr(0,line.find(",")))
            { 
                // output performance rule of data at the end of the design data line:
                std::ostringstream oss;
                int lastIndex =  mData.size()-1;
                for(int j=0; j < sdBest.size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBest(j) << ",";} 
                for(int j=0; j < bpBest.size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBest(j) << ",";} 
                for(int j=0; j < (*(mData[lastIndex])).size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*(mData[lastIndex]))(j) << ",";} // performance of last design of run
                oss << sdBestIteration << "," ;
                oss << bpBestIteration << "," ;
                oss << SDLastImprovementPercentage << "," ;
                oss << BPLastImprovementPercentage << "," ;
                oss << SDBestImprovementPercentage << "," ;
                oss << BPBestImprovementPercentage << "," ;
                
                lineFound = true;

                // Write the modified line to the temporary file
                line += oss.str();
                outputFile << line << std::endl;
                break;
            }
            else
            {
                outputFile << line << std::endl;
            }
            lineCount++;
        }

        if(!lineFound)
        {
            // output performance rule of data:
            int lastIndex =  mData.size()-1;
            outputFile << designName << ",";
            for(int j=0; j < sdBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBest(j) << ",";} 
            for(int j=0; j < bpBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBest(j) << ",";} 
            for(int j=0; j < (*(mData[lastIndex])).size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*(mData[lastIndex]))(j) << ",";} // performance of last design of run
            outputFile << sdBestIteration << "," ;
            outputFile << bpBestIteration << "," ;
            outputFile << SDLastImprovementPercentage << "," ;
            outputFile << BPLastImprovementPercentage << "," ;
            outputFile << SDBestImprovementPercentage << "," ;
            outputFile << BPBestImprovementPercentage << "," ;
        }
    
        inputFile.close();
        outputFile.close();

        // Remove original file
        if (std::remove(fileName.c_str()) != 0) 
        {
            std::stringstream errorMessage;
                errorMessage << "\nError, while deleting original file.\n"
                                        << "(bso/utilities/data_plot.cpp). " << std::endl;
                throw std::invalid_argument(errorMessage.str());
        } 
    }
    

    // Rename the temporary file to the original file name
    if (std::rename("temp.txt", fileName.c_str()) != 0) 
    {
        std::stringstream errorMessage;
            errorMessage << "\nError, renaming temporary file.\n"
                                    << "(bso/utilities/data_plot.cpp). " << std::endl;
            throw std::invalid_argument(errorMessage.str());
    } 
    else 
    {            
        std::cout << "File processing complete and original file overwritten." << std::endl;
    }
    
} // addToCombinedDataFile()

void data_plot::plotScatter2D(std::string fileNameScatter /*= "scatterData"*/, std::string pngName /*="plotScatter.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/)
{
    if(fileNameScatter == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(!visualize && !writeToPNG)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, no plot will be generated since both booleans 'visualize' and 'writeToPNG' are false.\n"
										 << "Make sure either one or both are set to true when this function is called to get results.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(axisDataTypes.size() != 2)
    {
         std::stringstream errorMessage;
				errorMessage << "\nError, expected two axis types to be defined to know which data is which and to give the plot axis variables.\n"
										 << "The current amount of axisDataTypes is: " << axisDataTypes.size() << " and it contains these variables: " << std::endl;
                                         for(const auto i: axisDataTypes) errorMessage << i << "; ";
                                         errorMessage << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Generate the scatterData file if none is currently already existing
    if(scatterData == "")
    {
        this->writeDataToFile(mData, fileNameScatter,"scatterData");
    }   

    // Define part of the a terminal command to make a plot with GNUplot
    std::string plotDetails = "plot '" + fileNameScatter  + "' using 1:2 title '" + dataName + "'\"";
    std::string axisSetting = "set xlabel '" + axisDataTypes[0] + "'; set ylabel '" + axisDataTypes[1] + "';";

    // Perform the plot terminal command to either make a png file of the plot (i.e. bool writeToPNG) or make a visualization (i.e. bool visualize)
    if(visualize)
    {
        std::string TerminalCommand = "gnuplot -persist -e \"set terminal x11; " + axisSetting + plotDetails;

        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << fileNameScatter << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
    if(writeToPNG)
    {
        std::string TerminalCommand = "gnuplot -e \"set terminal png; set output '" + pngName + "'; " + axisSetting + plotDetails;
        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << fileNameScatter << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
}

void data_plot::plotParetoFront2D(std::string fileNamePareto /*= "Pareto"*/, std::string fileNameNonPareto /*= "NonPareto"*/, std::string pngName /*="plot.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/)
{    
    if(fileNamePareto == "" || fileNameNonPareto == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(!visualize && !writeToPNG)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, no plot will be generated since both booleans 'visualize' and 'writeToPNG' are false.\n"
										 << "Make sure either one or both are set to true when this function is called to get results.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(axisDataTypes.size() != 2)
    {
         std::stringstream errorMessage;
				errorMessage << "\nError, expected two axis types to be defined to know which data is which and to give the plot axis variables.\n"
										 << "The current amount of axisDataTypes is: " << axisDataTypes.size() << " and it contains these variables: " << std::endl;
                                         for(const auto i: axisDataTypes) errorMessage << i << "; ";
                                         errorMessage << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    
    if(paretoData == "" || nonParetoData == "")
    {
        if(mData.size() == 0)
        {
            std::stringstream errorMessage;
                    errorMessage << "\nError, no data is stored within mData (i.e. this object) to plot. \n"
                                            << "Make sure there is data stored in mData. \n"
                                            << "(bso/utilities/data_plot.cpp). " << std::endl;
                    throw std::invalid_argument(errorMessage.str());
        }
        
        // Separate data in pareto front and non-pareto front
        std::vector<std::shared_ptr<data_point> > dataPareto;
        std::vector<std::shared_ptr<data_point> > dataNonPareto;
        this->getParetoFrontData(mData, dataPareto, dataNonPareto);
        
        // Write the pareto and non-pareto data to files
        this->writeDataToFile(dataPareto, fileNamePareto, "paretoData");
        this->writeDataToFile(dataNonPareto, fileNameNonPareto, "nonParetoData");
    }

    // Make the plot part of the terminal command to make a plot with GNUplot
    std::string plotDetails = "plot '" + fileNameNonPareto  + "' using 1:2 title '" + dataName + "', '" + fileNamePareto + "' using 1:2 title '" + dataName + " on pareto'\"";
    std::string axisSetting = "set xlabel '" + axisDataTypes[0] + "'; set ylabel '" + axisDataTypes[1] + "';";

    // Perform the plot terminal command to either make a png file of the plot (i.e. bool writeToPNG) or make a visualization (i.e. bool visualize)
    if(visualize)
    {
        std::string TerminalCommand = "gnuplot -persist -e \"set terminal x11; " + axisSetting + plotDetails;

        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << fileNameNonPareto << " and  " << fileNameNonPareto << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
    if(writeToPNG)
    {
        if(pngName == "")
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, no name for the to generate png has been given. \n"
										 << "Therefore, the png can not be generated.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
        
        std::string TerminalCommand = "gnuplot -e \"set terminal png; set output '" + pngName + "'; " + axisSetting + plotDetails;
        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << fileNameNonPareto << " and  " << fileNameNonPareto << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
}

void data_plot::plotVector2D(std::string fileNameVector /*= "vectorStyleData"*/, std::string pngName /*="plotVector.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/)
{    
    if(fileNameVector == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(!visualize && !writeToPNG)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, no plot will be generated since both boolians 'visualize' and 'writeToPNG' are false.\n"
										 << "Make sure either one or both are set to true when this function is called to get results.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(axisDataTypes.size() != 2)
    {
         std::stringstream errorMessage;
				errorMessage << "\nError, expected two axis types to be defined to know which data is which and to give the plot axis variables.\n"
										 << "The current amount of axisDataTypes is: " << axisDataTypes.size() << " and it contains these variables: " << std::endl;
                                         for(const auto i: axisDataTypes) errorMessage << i << "; ";
                                         errorMessage << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Make a data file with four columns where the first datapoint (x,y) needs to connect to the next two datapoint on the line (xdelta,ydelta) link:http://gnuplot.info/docs_6.0/loc6150.html
    if(vectorStyleData == "")
    {
        this->writeVectorStyleFile(mData, fileNameVector);
    }
    
    // Send a terminal command to make a plot with GNUplot
    std::string plotDetails = "plot '" + fileNameVector  + "' using 1:2:3:4 with vectors filled head title '" + dataName + " progression'\"";
    std::string axisSetting = "set xlabel '" + axisDataTypes[0] + "'; set ylabel '" + axisDataTypes[1] + "';";

    // Perform the plot terminal command to either make a png file of the plot (i.e. bool writeToPNG) or make a visualization (i.e. bool visualize)
    if(visualize)
    {
        std::string TerminalCommand = "gnuplot -persist -e \"set terminal x11; " + axisSetting + plotDetails;

        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << fileNameVector << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
    if(writeToPNG)
    {
        std::string TerminalCommand = "gnuplot -e \"set terminal png; set output '" + pngName + "'; " + axisSetting + plotDetails;
        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << fileNameVector << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
} // plotArrow2D()

void data_plot::getParetoFrontData(std::vector<std::shared_ptr<data_point> >& data, std::vector<std::shared_ptr<data_point> >& dataPareto, std::vector<std::shared_ptr<data_point> >& dataNonPareto)
{
    if(data.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, there is no data input given for the separation of data into Pareto and non pareto data.\n"
										 << "The size of data was found: " << data.size() << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    
    for(const auto& i: data)
    {
        bool isDominated = false;
        for(const auto& j: data)
        {
            if(i->isDominatedBy(*j))
            {
                isDominated = true;
                dataNonPareto.push_back(i);
                break;
            }
        }
        if(!isDominated)
        {
            dataPareto.push_back(i);
        }
    }
} // getParetoFrontData()

} // namespace utilities
} // namespace bso

#endif // BSO_DATA_PLOT_CPP