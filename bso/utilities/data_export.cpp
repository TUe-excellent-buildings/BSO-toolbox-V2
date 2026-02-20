#ifndef BSO_EXPORT_CPP
#define BSO_EXPORT_CPP

#include <fstream>
#include <string>
#include <sstream>

namespace bso { namespace utilities {

data_export::data_export()
{

} // ctor()

data_export::data_export(const std::vector<data_point>& data, std::vector<std::string> axisDataTypes, std::string plotName)
:   axisDataTypes(axisDataTypes), dataName(plotName)
{
	for (const auto& i : data)
	{
		mData.push_back(std::make_shared<data_point>(i));
	}
} // ctor()

data_export::data_export(utilities::data_export& rhs)
{
	mData = rhs.mData;
    axisDataTypes = rhs.axisDataTypes;
	dataName = rhs.dataName;
	scatterData = rhs.scatterData;
	paretoData = rhs.paretoData;
	vectorStyleData = rhs.vectorStyleData;
} // ctor()

data_export::~data_export()
{
	
} // dtor()

void data_export::writeDataToFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName, std::string dataKind /*= ""*/)
{ 
    if(fileName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(data.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected some data to be stored within the input variable data\n"
										 << "The found size of data is: " << data.size() << "\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
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
								 << "(bso/utilities/data_export.cpp). " << std::endl;
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
        for(unsigned int j=0; j< (*i).size(); j++)
        {     
            output << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*i)(j) << "\t";
        }
        output << std::endl;
    }

    if(dataKind == "scatterData") {scatterData = fileName;}
    if(dataKind == "paretoData") {paretoData = fileName;}
    if(dataKind == "nonParetoData") {nonParetoData = fileName;}
} // writeDataToFile()

void data_export::writeVectorStyleFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName)
{
    if(fileName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(data.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected some data to be stored within the input variable data\n"
										 << "The found size of data is: " << data.size() << "\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
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
								 << "(bso/utilities/data_export.cpp). " << std::endl;
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

            for(unsigned int j=0; j< (*i).size(); j++)
            {
                output  << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*i)(j) << "\t";
            }
            for(unsigned int j=0; j< (*next_i).size(); j++)
            {
                output << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << ((*next_i)(j)-(*i)(j)) << "\t";
            }
            output << std::endl;
        }
    }

    // Store the just generated file in object
    vectorStyleData = fileName;
} // writeVectorStyleFile()

void data_export::dataPointToStream(const utilities::data_point& data, std::ostringstream& oss) 
{
    for (unsigned int j=0; j < data.size(); j++) 
    {
        oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << data(j) << ",";
    }
}

template <typename T>
void data_export::vectorToStream(const std::vector<T>& data, unsigned int& iteration, const unsigned int& nrIterations, const int switchLineCount, const int lineCount, std::ostringstream& oss) 
{
    if(switchLineCount > lineCount)
    {
        oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << data[iteration] << ",";
        iteration++;
    }
    else if(iteration <= nrIterations)
    {
        oss << ","; /*insert blanks for the places where no values where found*/
        iteration++;
    }
}

template <typename T>
void data_export::initVectorToFile(const std::vector<T>& data, const int& nrIterations, std::ofstream& output)
{
    int iteration=0;
    for(const auto& i: data)
    {
        output << iteration << ",";
        output << i << ","<< std::endl;
        iteration++;
    }
    while(iteration <= nrIterations)
    {
        output << iteration << ",," << std::endl; // insert blanks for the places where no values where found
        iteration++;
    }
}

void data_export::addToCombinedDataFile(const std::string& fileName, const std::string& runKind, const std::string& designName, const unsigned int& nrIterations, const std::vector<bso::spatial_design::ms_N_building>& msDesigns, const std::vector<double>& sd_total_load, const std::vector<double>& exteriorWallArea)
{ 
    if(fileName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(designName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a designName with at least one character, but the designName name did not had any.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(runKind == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a runKind with at least one character, but the runKind name did not had any.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Check ouput file:
    std::ifstream inputFile(fileName);
    std::ofstream outputFile("temp.txt");
    if (!outputFile.is_open())
    { 
        std::stringstream errorMessage;
				errorMessage << "\nError, unable to open file.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Get relevant data to write to file:
    // Make a normalized copy for space iterations:
    std::vector<utilities::data_point> mDataNormalized;
    for(const auto& i: mData)
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
    utilities::data_point sdBest;
    utilities::data_point sdBestNormalized;
    utilities::data_point bpBest;
    utilities::data_point bpBestNormalized;
    utilities::data_point kneePoint;
    utilities::data_point kneePointObjective({0,1});
    std::vector<data_point> kneePointVectorNor;
    double SDBestImprovementPercentage;
    double BPBestImprovementPercentage;
    double SDLastImprovementPercentage;
    double BPLastImprovementPercentage;
    if(mData.size() != 0)
    {
        sdBest = *(mData[0]);
        sdBestNormalized = mDataNormalized[0];
        bpBest = *(mData[0]);
        bpBestNormalized = mDataNormalized[0];
        kneePointVectorNor = kneePointObjective.findClosestIn(mDataNormalized);
        for(long unsigned int i=0; i < mData.size(); i++)
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
        SDBestImprovementPercentage = (1-((*(mData[sdBestIteration]))(0)/(*(mData[0]))(0)))*100;
        BPBestImprovementPercentage = (1-((*(mData[bpBestIteration]))(1)/(*(mData[0]))(1)))*100;
        SDLastImprovementPercentage = (1-((*(mData[mData.size()-1]))(0)/(*(mData[0]))(0)))*100;
        BPLastImprovementPercentage = (1-((*(mData[mData.size()-1]))(1)/(*(mData[0]))(1)))*100;
    }

    // Generate output data where the index is equal to the considered iteration in msDesigns 
    std::vector<double> averageRecDif;
    std::vector<double> ratioBiggestSmallestFloorAreaSpace;
    std::vector<double> nrElevations;
    std::vector<double> median;
    std::vector<double> firstQuartile;
    std::vector<double> thirdQuartile;
    std::vector<double> average;
    std::vector<bool> isFragmented;
    std::vector<double> outliers;

    // Temporary storage for the data
    double medianTemp;
    double firstQuartileTemp;
    double thirdQuartileTemp;
    double averageTemp;
    std::vector<double> outliersTemp;

    // Get the above data
    for(auto& design: msDesigns)
    {
        averageRecDif.push_back(design.getAverageRecDif());
        ratioBiggestSmallestFloorAreaSpace.push_back(design.getSpaceFloorAreaRatio());
        nrElevations.push_back(design.getNrElevations());
        design.getSpaceFloorAreaBoxPlotValues(medianTemp, firstQuartileTemp, thirdQuartileTemp, averageTemp, outliersTemp);
        median.push_back(medianTemp);
        firstQuartile.push_back(firstQuartileTemp);
        thirdQuartile.push_back(thirdQuartileTemp);
        average.push_back(averageTemp);
        std::vector<bso::spatial_design::ms_space*> spacesNonBiggestFragment;
        isFragmented.push_back(design.isFragmented(spacesNonBiggestFragment));
        for(const auto& i: outliersTemp){outliers.push_back(i);}
    }

    // output the data to a file
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
        unsigned int iteration = 0;
        
        // Generate data and empty line if nr. of iteration was not reached
        for(const auto& i: mData)
        {
            outputFile << iteration << ",";
            for(int j=0; j< axisDataTypes.size(); j++)
            {     
                outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*i)(j) << ",";
            }
            outputFile << std::endl;
            iteration++;
        }
        while(iteration <= nrIterations)
        {
            outputFile << iteration << ",";
            for(int j=0; j< axisDataTypes.size(); j++)
            {     
                outputFile << ","; // insert blanks for the places where no values where found
            }
            outputFile << std::endl;
            iteration++;
        }
        outputFile << "kneePoint,";
        if(mData.size() == 0){for(long unsigned int j=0; j< axisDataTypes.size(); j++){outputFile << ",";}}
        else{for(long unsigned int j=0; j< kneePoint.size(); j++){outputFile << kneePoint(j) << ",";}}
        outputFile << std::endl;
        outputFile << "sdBest,";
        if(mData.size() == 0){for(int j=0; j< axisDataTypes.size(); j++){outputFile << ",";}}
        else{for(unsigned int j=0; j< sdBest.size(); j++){outputFile << sdBest(j) << ",";}}
        outputFile << std::endl;
        outputFile << "bpBest,";
        if(mData.size() == 0){for(long unsigned int j=0; j< axisDataTypes.size(); j++){outputFile << ",";}}
        else{for(unsigned int j=0; j< bpBest.size(); j++){outputFile << bpBest(j) << ",";}}
        outputFile << std::endl;
        if(mData.size() == 0)
        {
            outputFile << "kneePoint iteration," << ",," << std::endl;
            outputFile << "sdBest iteration," << ",," << std::endl;
            outputFile << "bpBest iteration," << ",," << std::endl;
            outputFile << "SD last iteration vs. first iteration," << ",," << std::endl;
            outputFile << "BP last iteration vs. first iteration," << ",," << std::endl;
            outputFile << "SD best iteration vs. first iteration," << ",," << std::endl;
            outputFile << "BP best iteration vs. first iteration," << ",," << std::endl;

            // output the msDesign data:
            outputFile << "," << designName << runKind << std::endl;
            outputFile << "MS Design data,Recdif" << std::endl;
            std::vector<double> emptyDataVector;
            initVectorToFile(emptyDataVector, nrIterations, outputFile);
            outputFile << ",Ratio biggest vs. smallest floor area space" << std::endl;
            initVectorToFile(emptyDataVector, nrIterations, outputFile);
            outputFile << ",nrElevations" << std::endl;
            initVectorToFile(emptyDataVector, nrIterations, outputFile);
            outputFile << ",total loads" << std::endl;
            initVectorToFile(emptyDataVector, nrIterations, outputFile);
            outputFile << ",exterior surface area," << std::endl;
            initVectorToFile(emptyDataVector, nrIterations, outputFile);
            outputFile << ",median," << std::endl;
            initVectorToFile(emptyDataVector, nrIterations, outputFile);
            outputFile << ",firstQuartile," << std::endl;
            initVectorToFile(emptyDataVector, nrIterations, outputFile);
            outputFile << ",thirdQuartile," << std::endl;
            initVectorToFile(emptyDataVector, nrIterations, outputFile);
            outputFile << ",thirdQuartile," << std::endl;
            initVectorToFile(emptyDataVector, nrIterations, outputFile);
            outputFile << ",average," << std::endl;
            initVectorToFile(emptyDataVector, nrIterations, outputFile);
            outputFile << ",isFragmented" << std::endl;
            initVectorToFile(emptyDataVector, nrIterations, outputFile);
            // initVectorToFile(emptyDataVector, nrIterations, outputFile);
        }
        else
        {
            outputFile << "kneePoint iteration," << kneePointIteration << ",," << std::endl;
            outputFile << "sdBest iteration," << sdBestIteration << ",," << std::endl;
            outputFile << "bpBest iteration," << bpBestIteration << ",," << std::endl;
            outputFile << "SD last iteration vs. first iteration," << SDLastImprovementPercentage << ",," << std::endl;
            outputFile << "BP last iteration vs. first iteration," << BPLastImprovementPercentage << ",," << std::endl;
            outputFile << "SD best iteration vs. first iteration," << SDBestImprovementPercentage << ",," << std::endl;
            outputFile << "BP best iteration vs. first iteration," << BPBestImprovementPercentage << ",," << std::endl;

            // output the msDesign data:
            outputFile << "\n," << designName << runKind << std::endl;
            outputFile << "MS Design data,Recdif" << std::endl;
            initVectorToFile(averageRecDif, nrIterations, outputFile);
            outputFile << ",Ratio biggest vs. smallest floor area space" << std::endl;
            initVectorToFile(ratioBiggestSmallestFloorAreaSpace, nrIterations, outputFile);
            outputFile << ",nrElevations" << std::endl;
            initVectorToFile(nrElevations, nrIterations, outputFile);
            outputFile << ",total loads" << std::endl;
            initVectorToFile(sd_total_load, nrIterations, outputFile);
            outputFile << ",exterior surface area" << std::endl;
            initVectorToFile(exteriorWallArea, nrIterations, outputFile);
            outputFile << ",median" << std::endl;
            initVectorToFile(median, nrIterations, outputFile);
            outputFile << ",firstQuartile" << std::endl;
            initVectorToFile(firstQuartile, nrIterations, outputFile);
            outputFile << ",thirdQuartile" << std::endl;
            initVectorToFile(thirdQuartile, nrIterations, outputFile);
            outputFile << ",average" << std::endl;
            initVectorToFile(average, nrIterations, outputFile);
            outputFile << ",isFragmented" << std::endl;
            initVectorToFile(isFragmented, nrIterations, outputFile);
        }

        inputFile.close();
        outputFile.close();
    } 
    else 
    {
        // File is not empty, process existing lines
        unsigned int lineCount = 0;
        unsigned int dataCount = 0;
        unsigned int iteration = 0;
        std::string line;
        while(getline(inputFile, line)) 
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
                for(int j=0; j< axisDataTypes.size(); j++)
                {     
                    // Use ostringstream to format the double with full precision
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*(mData[dataCount]))(j) << ",";
                    // Append the formatted value to the line
                    line += oss.str();
                }
                dataCount++;
            }
            else if(dataCount <= nrIterations)
            {
                for(long unsigned int j=0; j< axisDataTypes.size(); j++)
                {     
                    // Use ostringstream to format the double with full precision
                    std::ostringstream oss;
                    oss << ",";
                    // Append the formatted value to the line
                    line += oss.str();
                }
                dataCount++;
            }
            else if(mData.size() == 0)
            {
                std::ostringstream oss;
                while(dataCount+11 != lineCount)
                {
                    for(long unsigned int j=0; j< axisDataTypes.size(); j++){oss << ",";}
                    lineCount++;
                }
                line += oss.str();
            }
            else
            {
                std::ostringstream oss;

                if (dataCount + 2 == lineCount) {dataPointToStream(kneePoint, oss);}
                else if (dataCount + 3 == lineCount) {dataPointToStream(sdBest, oss);} 
                else if (dataCount + 4 == lineCount) {dataPointToStream(bpBest, oss);}
                else if (dataCount + 5 == lineCount) {oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << kneePointIteration << ",,";}
                else if (dataCount + 6 == lineCount) {oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBestIteration << ",,";}
                else if (dataCount + 7 == lineCount) {oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBestIteration << ",,";} 
                else if (dataCount + 8 == lineCount) {oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << SDLastImprovementPercentage << ",,";} 
                else if (dataCount + 9 == lineCount) {oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << BPLastImprovementPercentage << ",,";} 
                else if (dataCount + 10 == lineCount) {oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << SDBestImprovementPercentage << ",,";} 
                else if (dataCount + 11 == lineCount) {oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << BPBestImprovementPercentage << ",,";} 
                else if (dataCount + 12 == lineCount) {/*do nothing since empty line*/}
                else if (dataCount + 13 == lineCount) {oss << "," << designName << runKind ;}
                else if(dataCount+14 == lineCount){oss << ",Recdif" ;}
                else if(dataCount+15 + nrIterations >= lineCount){vectorToStream(averageRecDif, iteration, nrIterations, msDesigns.size() + dataCount+15 , lineCount, oss);}
                else if(dataCount+16 + nrIterations == lineCount){oss << ",Ratio biggest vs. smallest floor area space"; iteration = 0;}
                else if(dataCount+17 + nrIterations*2 >= lineCount){vectorToStream(ratioBiggestSmallestFloorAreaSpace, iteration, nrIterations, msDesigns.size() + dataCount+17 + nrIterations, lineCount, oss);}
                else if(dataCount+18 + nrIterations*2 == lineCount){oss << ",nrElevations"; iteration = 0;}
                else if(dataCount+19 + nrIterations*3 >= lineCount){vectorToStream(nrElevations, iteration, nrIterations, msDesigns.size() + dataCount+19 + nrIterations*2 , lineCount, oss);}
                else if(dataCount+20 + nrIterations*3 == lineCount){oss << ",total loads"; iteration = 0;}
                else if(dataCount+21 + nrIterations*4 >= lineCount){vectorToStream(sd_total_load, iteration, nrIterations, msDesigns.size() + dataCount+21 + nrIterations*3 , lineCount, oss);}
                else if(dataCount+22 + nrIterations*4 == lineCount){oss << ",exterior surface area"; iteration = 0;}
                else if(dataCount+23 + nrIterations*5 >= lineCount){vectorToStream(exteriorWallArea, iteration, nrIterations, msDesigns.size() + dataCount+23 + nrIterations*4 , lineCount, oss);}
                else if(dataCount+24 + nrIterations*5 == lineCount){oss << ",median"; iteration = 0;}
                else if(dataCount+25 + nrIterations*6 >= lineCount){vectorToStream(median, iteration, nrIterations, msDesigns.size() + dataCount+25 + nrIterations*5 , lineCount, oss);}
                else if(dataCount+26 + nrIterations*6 == lineCount){oss << ",firstQuartile"; iteration = 0;}
                else if(dataCount+27 + nrIterations*7 >= lineCount){vectorToStream(firstQuartile, iteration, nrIterations, msDesigns.size() + dataCount+27 + nrIterations*6 , lineCount, oss);}
                else if(dataCount+28 + nrIterations*7 == lineCount){oss << ",thirdQuartile"; iteration = 0;}
                else if(dataCount+29 + nrIterations*8 >= lineCount){vectorToStream(thirdQuartile, iteration, nrIterations, msDesigns.size() + dataCount+29 + nrIterations*7 , lineCount, oss);}
                else if(dataCount+30 + nrIterations*8 == lineCount){oss << ",average"; iteration = 0;}
                else if(dataCount+31 + nrIterations*9 >= lineCount){vectorToStream(average, iteration, nrIterations, msDesigns.size() + dataCount+31 + nrIterations*8 , lineCount, oss);}
                else if(dataCount+32 + nrIterations*9 == lineCount){oss << ",ifragmented"; iteration = 0;}
                else if(dataCount+33 + nrIterations*10 >= lineCount){vectorToStream(isFragmented, iteration, nrIterations, msDesigns.size() + dataCount+33 + nrIterations*9 , lineCount, oss);}
                
                // Add outliers output to the oss export. Important since there are more outliers per iterations that they are outputted per iteration in the file


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
                                        << "(bso/utilities/data_export.cpp). " << std::endl;
                throw std::invalid_argument(errorMessage.str());
        } 
    }
    
    // Rename the temporary file to the original file name
    if (std::rename("temp.txt", fileName.c_str()) != 0) 
    {
        std::stringstream errorMessage;
            errorMessage << "\nError, renaming temporary file.\n"
                                    << "(bso/utilities/data_export.cpp). " << std::endl;
            throw std::invalid_argument(errorMessage.str());
    } 
    else 
    {            
        std::cout << "In addToCombinedDataFile(): File processing complete and temp file successfully renamed." << std::endl;
    }
    
} // addToCombinedDataFile()

void data_export::addToOverviewCombinedDataFile(const std::string& fileName, const std::string& runKind, const std::string& designName, const unsigned int numberOfIterationsPerformed, const bso::spatial_design::ms_N_building& initialDesign)
{ 
    if(fileName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(designName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a designName with at least one character, but the designName name did not had any.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(runKind == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a runKind with at least one character, but the runKind name did not had any.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Check ouput file:
    std::ifstream inputFile(fileName);
    std::ofstream outputFile("temp.txt");
    if (!outputFile.is_open())
    { 
        std::stringstream errorMessage;
				errorMessage << "\nError, unable to open file.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Get relevant data to write to file:
    // Make a normalized copy for space iterations:
    std::vector<utilities::data_point> mDataNormalized;
    for(const auto& i: mData)
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
    for(long unsigned int i=0; i < mData.size(); i++)
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
        // initial design data:
        outputFile << ",Nr spaces,Floor area,Nr.Elevation,Ratio biggest vs. smallest space,RecDif,";

        // run data
        outputFile << "best sd design in sd run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "best bp design in sd run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "Last design of sd run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        for(int j=0; j < 7; j++) outputFile << ",";

        outputFile << "best bp design in bp run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "best sd design in bp run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "Last design of bp run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        for(int j=0; j < 7; j++) outputFile << ",";

        outputFile << "best sd design in weighted run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "best bp design in weighted run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << "Last design of weighted run";
        for(std::string i: axisDataTypes) { outputFile << ",";}
        outputFile << std::endl;
        
        // make rule with value indicators:
        for(int i=0; i<6; i++) {outputFile << ",";}
        for(int j=0; j < 3; j++)
        {
            for(std::string i: axisDataTypes)
            {
                outputFile <<  i  + ",";
            }
        }
        outputFile << "sd best iteration,bp best iteration,sd improvement last vs. initial,bp improvement last vs. initial,sd improvement best vs. initial,bp improvement best vs. initial,number Of iteration Performed,";
        for(int j=0; j < 3; j++)
        {
            for(std::string i: axisDataTypes)
            {
                outputFile <<  i  + ",";
            }
        }
        outputFile << "sd best iteration,bp best iteration,sd improvement last vs. initial,bp improvement last vs. initial,sd improvement best vs. initial,bp improvement best vs. initial,number Of iteration Performed,";
        for(int j=0; j < 3; j++)
        {
            for(std::string i: axisDataTypes)
            {
                outputFile <<  i  + ",";
            }
        }
        outputFile << "sd best iteration,bp best iteration,sd improvement last vs. initial,bp improvement last vs. initial,sd improvement best vs. initial,bp improvement best vs. initial,number Of iteration Performed,";
        outputFile << std::endl;

        // output performance rule of data:
        if(mData.size() == 0)
        {
            // output enough ',' to fill the space of the run
            outputFile << designName << ",";
            outputFile << initialDesign.getSpacePtrs().size() << ",";
            outputFile << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getFloorArea() << ",";
            outputFile << initialDesign.getNrElevations() << ",";
            outputFile << initialDesign.getSpaceFloorAreaRatio() << ",";
            outputFile << initialDesign.getAverageRecDif() << ",";
            for(long unsigned int j=0; j < axisDataTypes.size(); j++){outputFile << ",";} 
            for(long unsigned int j=0; j < axisDataTypes.size(); j++){outputFile << ",";} 
            for(long unsigned int j=0; j < axisDataTypes.size(); j++){outputFile << ",";}
            for(long unsigned int j=0; j < 7; j++) outputFile << ",";
        }
        else
        {
            int lastIndex =  mData.size()-1;
            outputFile << designName << ",";
            outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getSpacePtrs().size() << ",";
            outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getFloorArea() << ",";
            outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getNrElevations() << ",";
            outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getSpaceFloorAreaRatio() << ",";
            outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getAverageRecDif() << ",";
            for(unsigned int j=0; j < sdBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBest(j) << ",";} 
            for(unsigned int j=0; j < bpBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBest(j) << ",";} 
            for(unsigned int j=0; j < (*(mData[lastIndex])).size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*(mData[lastIndex]))(j) << ",";} // performance of last design of run
            outputFile << sdBestIteration << "," ;
            outputFile << bpBestIteration << "," ;
            outputFile << SDLastImprovementPercentage << "," ;
            outputFile << BPLastImprovementPercentage << "," ;
            outputFile << SDBestImprovementPercentage << "," ;
            outputFile << BPBestImprovementPercentage << "," ;
            outputFile << numberOfIterationsPerformed << "," ;
        }

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
                if(mData.size() == 0)
                {
                    // output enough ',' to fill the space of the run
                    for(long unsigned int j=0; j < axisDataTypes.size(); j++){outputFile << ",";} 
                    for(long unsigned int j=0; j < axisDataTypes.size(); j++){outputFile << ",";} 
                    for(long unsigned int j=0; j < axisDataTypes.size(); j++){outputFile << ",";}
                    for(long unsigned int j=0; j < 7; j++) outputFile << ",";
                }
                else
                {
                    int lastIndex =  mData.size()-1;
                    for(unsigned int j=0; j < sdBest.size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBest(j) << ",";} 
                    for(unsigned int j=0; j < bpBest.size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBest(j) << ",";} 
                    for(unsigned int j=0; j < (*(mData[lastIndex])).size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*(mData[lastIndex]))(j) << ",";} // performance of last design of run
                    oss << sdBestIteration << "," ;
                    oss << bpBestIteration << "," ;
                    oss << SDLastImprovementPercentage << "," ;
                    oss << BPLastImprovementPercentage << "," ;
                    oss << SDBestImprovementPercentage << "," ;
                    oss << BPBestImprovementPercentage << "," ;
                    oss << numberOfIterationsPerformed << "," ;
                }
                
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
            if(mData.size() == 0)
            {
                // output enough ',' to fill the space of the run
                outputFile << designName << ",";
                outputFile << initialDesign.getSpacePtrs().size() << ",";
                outputFile << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getFloorArea() << ",";
                outputFile << initialDesign.getNrElevations() << ",";
                outputFile << initialDesign.getSpaceFloorAreaRatio() << ",";
                outputFile << initialDesign.getAverageRecDif() << ",";
                for(long unsigned int j=0; j < axisDataTypes.size(); j++){outputFile << ",";} 
                for(long unsigned int j=0; j < axisDataTypes.size(); j++){outputFile << ",";} 
                for(long unsigned int j=0; j < axisDataTypes.size(); j++){outputFile << ",";}
                for(int j=0; j < 7; j++) outputFile << ",";
            }
            else
            {
                int lastIndex =  mData.size()-1;
                outputFile << designName << ",";
                outputFile << initialDesign.getSpacePtrs().size() << ",";
                outputFile << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getFloorArea() << ",";
                outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getNrElevations() << ",";
                outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getSpaceFloorAreaRatio() << ",";
                outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getAverageRecDif() << ",";
                for(unsigned int j=0; j < sdBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBest(j) << ",";} 
                for(unsigned int j=0; j < bpBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBest(j) << ",";} 
                for(unsigned int j=0; j < (*(mData[lastIndex])).size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*(mData[lastIndex]))(j) << ",";} // performance of last design of run
                outputFile << sdBestIteration << "," ;
                outputFile << bpBestIteration << "," ;
                outputFile << SDLastImprovementPercentage << "," ;
                outputFile << BPLastImprovementPercentage << "," ;
                outputFile << SDBestImprovementPercentage << "," ;
                outputFile << BPBestImprovementPercentage << "," ;
                outputFile << numberOfIterationsPerformed << "," ;
            }
        }
    
        inputFile.close();
        outputFile.close();

        // Remove original file
        if (std::remove(fileName.c_str()) != 0) 
        {
            std::stringstream errorMessage;
                errorMessage << "\nError, while deleting original file.\n"
                                        << "(bso/utilities/data_export.cpp). " << std::endl;
                throw std::invalid_argument(errorMessage.str());
        } 
    }
    

    // Rename the temporary file to the original file name
    if (std::rename("temp.txt", fileName.c_str()) != 0) 
    {
        std::stringstream errorMessage;
            errorMessage << "\nError, renaming temporary file.\n"
                                    << "(bso/utilities/data_export.cpp). " << std::endl;
            throw std::invalid_argument(errorMessage.str());
    } 
    else 
    {            
        std::cout << "In addToOverviewCombinedDataFile: File processing complete and temp file successfully renamed." << std::endl;
    }
    
} // addToOverviewCombinedDataFile()

void data_export::plotScatter2D(std::string fileNameScatter /*= "scatterData"*/, std::string pngName /*="plotScatter.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/)
{
    if(fileNameScatter == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(!visualize && !writeToPNG)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, no plot will be generated since both booleans 'visualize' and 'writeToPNG' are false.\n"
										 << "Make sure either one or both are set to true when this function is called to get results.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(axisDataTypes.size() != 2)
    {
         std::stringstream errorMessage;
				errorMessage << "\nError, expected two axis types to be defined to know which data is which and to give the plot axis variables.\n"
										 << "The current amount of axisDataTypes is: " << axisDataTypes.size() << " and it contains these variables: " << std::endl;
                                         for(const auto& i: axisDataTypes) errorMessage << i << "; ";
                                         errorMessage << "(bso/utilities/data_export.cpp). " << std::endl;
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
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
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
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
}

void data_export::plotParetoFront2D(std::string fileNamePareto /*= "Pareto"*/, std::string fileNameNonPareto /*= "NonPareto"*/, std::string pngName /*="plot.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/)
{    
    if(fileNamePareto == "" || fileNameNonPareto == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(!visualize && !writeToPNG)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, no plot will be generated since both booleans 'visualize' and 'writeToPNG' are false.\n"
										 << "Make sure either one or both are set to true when this function is called to get results.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(axisDataTypes.size() != 2)
    {
         std::stringstream errorMessage;
				errorMessage << "\nError, expected two axis types to be defined to know which data is which and to give the plot axis variables.\n"
										 << "The current amount of axisDataTypes is: " << axisDataTypes.size() << " and it contains these variables: " << std::endl;
                                         for(const auto& i: axisDataTypes) errorMessage << i << "; ";
                                         errorMessage << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    
    if(paretoData == "" || nonParetoData == "")
    {
        if(mData.size() == 0)
        {
            std::stringstream errorMessage;
                    errorMessage << "\nError, no data is stored within mData (i.e. this object) to plot. \n"
                                            << "Make sure there is data stored in mData. \n"
                                            << "(bso/utilities/data_export.cpp). " << std::endl;
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
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
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
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
        
        std::string TerminalCommand = "gnuplot -e \"set terminal png; set output '" + pngName + "'; " + axisSetting + plotDetails;
        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << fileNameNonPareto << " and  " << fileNameNonPareto << "\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
}

void data_export::plotVector2D(std::string fileNameVector /*= "vectorStyleData"*/, std::string pngName /*="plotVector.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/)
{    
    if(fileNameVector == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(!visualize && !writeToPNG)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, no plot will be generated since both boolians 'visualize' and 'writeToPNG' are false.\n"
										 << "Make sure either one or both are set to true when this function is called to get results.\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(axisDataTypes.size() != 2)
    {
         std::stringstream errorMessage;
				errorMessage << "\nError, expected two axis types to be defined to know which data is which and to give the plot axis variables.\n"
										 << "The current amount of axisDataTypes is: " << axisDataTypes.size() << " and it contains these variables: " << std::endl;
                                         for(const auto& i: axisDataTypes) errorMessage << i << "; ";
                                         errorMessage << "(bso/utilities/data_export.cpp). " << std::endl;
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
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
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
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
} // plotArrow2D()

void data_export::getParetoFrontData(std::vector<std::shared_ptr<data_point> >& data, std::vector<std::shared_ptr<data_point> >& dataPareto, std::vector<std::shared_ptr<data_point> >& dataNonPareto)
{
    if(data.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, there is no data input given for the separation of data into Pareto and non pareto data.\n"
										 << "The size of data was found: " << data.size() << "\n"
                                         << "(bso/utilities/data_export.cpp). " << std::endl;
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

#endif // BSO_EXPORT_CPP