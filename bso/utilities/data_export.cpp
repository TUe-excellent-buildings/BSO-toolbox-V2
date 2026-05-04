#ifndef BSO_EXPORT_CPP
#define BSO_EXPORT_CPP

#include <fstream>
#include <string>
#include <sstream>

namespace bso { namespace utilities {

void data_export::addToCombinedDataFileParallel(const std::string& fileName, const std::string& runKind, const std::string& designName, const unsigned int& nrIterations, const std::vector<bso::spatial_design::ms_N_building>& msDesigns, const std::vector<double>& sd_total_load, const std::vector<double>& exteriorWallArea)
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
    if(mData.size() == 0)
    {
        if((*mData[0]).size() != mAxisDataTypes.size())
        {
            std::stringstream errorMessage;
            errorMessage << "\nError, expected the size of the data points in mData to be equal to the size of mAxisDataTypes \n"
                         << "to know which data is which and to give the plot axis variables.\n"
                         << "The found size of the data points in mData is: " << (*mData[0]).size() 
                         << "\nand it contains these variables: " << std::endl;
            for(const auto& i: mAxisDataTypes) errorMessage << i << "; ";
            errorMessage << "(bso/utilities/data_export.cpp). " << std::endl;
            throw std::invalid_argument(errorMessage.str());
        }
    }

    std::ofstream outputFile(fileName);
    if (!outputFile.is_open())
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, unable to open file.\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }

    // Get relevant data to write to file:
    std::vector<utilities::data_point> mDataNormalized;
    CombinedStats stats = computeCombinedStats(nrIterations, mDataNormalized);
    MsDesignData msData = collectMsDesignData(msDesigns);

    // Write headers and data (copy from addToCombinedDataFile 'file does not exist yet' branch)
    outputFile << "," << designName << "_" << runKind;
    for(int axisIndex = 1; axisIndex < mAxisDataTypes.size(); axisIndex++)
    {
        outputFile << ",";
    }
    outputFile << "," << std::endl << ",";
    for(std::string i: mAxisDataTypes)
    {
        outputFile << i << ",";
    }
    outputFile << std::endl;
    unsigned int iteration = 0;
    for(const auto& i: mData)
    {
        outputFile << iteration << ",";
        for(unsigned int j=0; j< mAxisDataTypes.size(); j++)
        {
            outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*i)(j) << ",";
        }
        outputFile << std::endl;
        iteration++;
    }
    while(iteration <= nrIterations)
    {
        outputFile << iteration << ",";
        for(unsigned int j=0; j< mAxisDataTypes.size(); j++)
        {
            outputFile << ",";
        }
        outputFile << std::endl;
        iteration++;
    }
    outputFile << "kneePoint,";
    if(mData.size() == 0){for(long unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}}
    else
    {
        for(long unsigned int j=0; j< stats.kneePoint.size(); j++){outputFile << stats.kneePoint(j) << ",";}
    }
    outputFile << std::endl;

    // Output a "<axisName>Best" row for each objective, using the corresponding bestDataPoints entry when available.
    for(long unsigned int obj = 0; obj < mAxisDataTypes.size(); ++obj)
    {
        outputFile << mAxisDataTypes[obj] << "Best,";
        if(mData.size() == 0)
        {
            for(long unsigned int j = 0; j < mAxisDataTypes.size(); ++j){outputFile << ",";}
        }
        else if(obj < stats.bestDataPoints.size())
        {
            for(unsigned int j = 0; j < stats.bestDataPoints[obj].size(); ++j){outputFile << stats.bestDataPoints[obj](j) << ",";}
        }
        else
        {
            for(long unsigned int j = 0; j < mAxisDataTypes.size(); ++j){outputFile << ",";}
        }
        outputFile << std::endl;
    }
    if(mData.size() == 0)
    {
        outputFile << "kneePoint iteration,";
        for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
        outputFile << std::endl;
        for(const auto objectiveName: mAxisDataTypes)
        {
            outputFile << objectiveName << "Best iteration,";
            for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
            outputFile<< std::endl;
        }
        for(const auto objectiveName: mAxisDataTypes)
        {
            outputFile << objectiveName << " last iteration vs. first iteration,";
            for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
            outputFile<< std::endl;
        }
        for(const auto objectiveName: mAxisDataTypes)
        {
            outputFile << objectiveName << " best iteration vs. first iteration,";
            for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
            outputFile<< std::endl;
        }

        // output the msDesign data:
        outputFile << "," << designName << runKind << std::endl;
        outputFile << "MS Design data,Recdif" << std::endl;
        std::vector<double> emptyDataVector;
        initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
        outputFile << ",Ratio biggest vs. smallest floor area space" << std::endl;
        initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
        outputFile << ",nrElevations" << std::endl;
        initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
        outputFile << ",total loads" << std::endl;
        initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
        outputFile << ",exterior surface area," << std::endl;
        initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
        outputFile << ",isFragmented" << std::endl;
        initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
    }
    else
    {
        outputFile << "kneePoint iteration," << stats.kneePointIteration;
        for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
        outputFile << std::endl;
        for(unsigned int index=0; index<mAxisDataTypes.size(); index++)
        {
            outputFile << mAxisDataTypes[index] << "Best iteration," << stats.bestIterations[index];
            for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
            outputFile << std::endl;
        }
        for(unsigned int index=0; index<mAxisDataTypes.size(); index++)
        {
            outputFile << mAxisDataTypes[index] << " last iteration vs. first iteration," << stats.lastImprovementPercentages[index];
            for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
            outputFile << std::endl;
        }
        for(unsigned int index=0; index<mAxisDataTypes.size(); index++)
        {
            outputFile << mAxisDataTypes[index] << " best iteration vs. first iteration," << stats.bestImprovementPercentages[index];
            for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
            outputFile << std::endl;
        }

        // output the msDesign data:
        outputFile << "\n," << designName << runKind << std::endl;
        outputFile << "MS Design data,Recdif" << std::endl;
        initVectorToFile(msData.averageRecDif, mAxisDataTypes, nrIterations, outputFile);
        outputFile << ",Ratio biggest vs. smallest floor area space" << std::endl;
        initVectorToFile(msData.ratioBiggestSmallestFloorAreaSpace, mAxisDataTypes, nrIterations, outputFile);
        outputFile << ",nrElevations" << std::endl;
        initVectorToFile(msData.nrElevations, mAxisDataTypes, nrIterations, outputFile);
        outputFile << ",total loads" << std::endl;
        initVectorToFile(sd_total_load, mAxisDataTypes, nrIterations, outputFile);
        outputFile << ",exterior surface area" << std::endl;
        initVectorToFile(exteriorWallArea, mAxisDataTypes, nrIterations, outputFile);
        outputFile << ",isFragmented" << std::endl;
        initVectorToFile(msData.isFragmented, mAxisDataTypes, nrIterations, outputFile);
    }

    outputFile.close();
}

data_export::data_export()
{

} // ctor()

data_export::data_export(const std::vector<data_point>& data, std::vector<std::string> axisDataTypes, std::string plotName)
:   mAxisDataTypes(axisDataTypes), mDataName(plotName)
{
	if(data[0].size() != mAxisDataTypes.size())
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected the size of the data points in data to be equal to the size of mAxisDataTypes \n"
                     << "to know which data is which and to give the plot axis variables.\n"
                     << "The found size of the data points in data is: " << data[0].size() 
                     << "\nand it contains these variables: " << std::endl;
                     for(const auto& i: mAxisDataTypes) errorMessage << i << "; ";
                     errorMessage << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    
    for (const auto& i : data)
	{
		mData.push_back(std::make_shared<data_point>(i));
	}
} // ctor()

data_export::data_export(utilities::data_export& rhs)
{
	mData = rhs.mData;
    mAxisDataTypes = rhs.mAxisDataTypes;
	mDataName = rhs.mDataName;
    mScatterData = rhs.mScatterData;
    mParetoData = rhs.mParetoData;
    mNonParetoData = rhs.mNonParetoData;
    mVectorStyleData = rhs.mVectorStyleData;
    mMultiAxisData = rhs.mMultiAxisData;
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
    for(std::string i: mAxisDataTypes)
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

    if(dataKind == "scatterData") {mScatterData = fileName;}
    if(dataKind == "paretoData") {mParetoData = fileName;}
    if(dataKind == "nonParetoData") {mNonParetoData = fileName;}
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
    for(std::string i: mAxisDataTypes)
    {
        output << i << "\t";
    }
    for(std::string i: mAxisDataTypes)
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
    mVectorStyleData = fileName;
} // writeVectorStyleFile()

void data_export::writeMultiAxisFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName)
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
    if(mAxisDataTypes.size() < 2)
    {
        std::stringstream errorMessage;
                errorMessage << "\nError, expected at least two axis types to be defined to know which data is which and to give the plot axis variables.\n"
                             << "The current amount of mAxisDataTypes is: " << mAxisDataTypes.size() << " and it contains these variables: " << std::endl;
                             for(const auto& i: mAxisDataTypes) errorMessage << i << "; ";
                             errorMessage << "(bso/utilities/data_export.cpp). " << std::endl;
                throw std::invalid_argument(errorMessage.str());
    }

    // Output mData to fileName in the format gnuPlot can read later on.
    std::ofstream output;
    if (!fileName.empty()) output.open(fileName.c_str());
	
    if (fileName.empty() || !output.is_open())
    {
        std::stringstream errorMessage;
        errorMessage << "Could not open the following file to write a multi-axis dataFile for plotting to:" << std::endl
                     << ((fileName.empty())? "no input file given" : fileName) << std::endl
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }

    // Header: x-column followed by all y-columns
    output << "# " << "iteration" << "\t";
    for(const std::string& axisName : mAxisDataTypes)
    {
        output << axisName << "\t";
    }
    output << std::endl;

    // Second header line: human-readable axis labels taken from mAxisLabels,
    // so downstream plotting scripts do not need to hard-code them.
    output << "# axis_labels ";
    bool firstLabel = true;

    // x-axis label (iteration)
    {
        const std::string key = "iteration";
        const std::string label = (mAxisLabels.count(key) ? mAxisLabels.at(key) : key);
        output << key << "=" << label;
        firstLabel = false;
    }

    // y-axis labels, in the same order as mAxisDataTypes
    for(const std::string& axisName : mAxisDataTypes)
    {
        const std::string label = (mAxisLabels.count(axisName) ? mAxisLabels.at(axisName) : axisName);
        output << (firstLabel ? "" : "; ") << axisName << "=" << label;
        firstLabel = false;
    }
    output << std::endl;

    // Data: each row contains all components of the data_point
    unsigned int iteration = 0;
    for(const auto& pointPtr : data)
    {
        const data_point& point = *pointPtr;
        output << iteration << "\t";
        for(unsigned int j = 0; j < point.size(); j++)
        {
            output << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << point(j) << "\t";
        }
        output << std::endl;
        iteration++;
    }

    // Store the just generated file in object
    mMultiAxisData = fileName;
}

void data_export::writeObjective3DFile(const std::string& fileName) const
{
    if(fileName == "")
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected a file name with at least one character, but the file name did not have any.\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if(mData.empty())
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected some data in mData to write a 3D objective file, but mData is empty.\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if(mAxisDataTypes.size() < 3)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected at least three axis types (sd, bp, mADFbuilding_inverse) to write a 3D objective file.\n"
                     << "Current axis types (" << mAxisDataTypes.size() << "): ";
        for(const auto& name : mAxisDataTypes) errorMessage << name << "; ";
        errorMessage << "\n(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }

    int sdIndex  = -1;
    int bpIndex  = -1;
    int dlIndex  = -1; // dl_ADFbuilding_inverse
    int dlScoreIndex = -1; // optional: dl_score (BS EN 17037-2018)
    for(std::size_t i = 0; i < mAxisDataTypes.size(); ++i)
    {
        const std::string& name = mAxisDataTypes[i];
        if(name == "sd") sdIndex = static_cast<int>(i);
        else if(name == "bp") bpIndex = static_cast<int>(i);
        else if(name == "dl_ADFbuilding_inverse") dlIndex = static_cast<int>(i);
        else if(name == "dl_score") dlScoreIndex = static_cast<int>(i);
    }
    if(sdIndex < 0 || bpIndex < 0 || dlIndex < 0)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, could not find required objective names 'sd', 'bp' and 'dl_ADFbuilding_inverse' in mAxisDataTypes.\n"
                     << "Current axis types: ";
        for(const auto& name : mAxisDataTypes) errorMessage << name << "; ";
        errorMessage << "\n(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }

    std::ofstream output(fileName.c_str());
    if(!output.is_open())
    {
        std::stringstream errorMessage;
        errorMessage << "Could not open the following file to write a 3D objective data file for plotting to:" << std::endl
                     << fileName << std::endl
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }

    // Compute knee-point iteration using existing helper.
    std::vector<utilities::data_point> mDataNormalized;
    CombinedStats stats = computeCombinedStats(static_cast<unsigned int>(mData.size() - 1), mDataNormalized);

    // Header: names and then knee point metadata.
    output << "# iteration\tsd\tbp\tdl_ADFbuilding_inverse" << std::endl;

    // Second header: knee-point iteration and axis labels from mAxisLabels.
    output << "# kneePointIteration " << stats.kneePointIteration;
    output << " axis_labels ";
    bool firstLabel3D = true;
    for(const auto& kv : mAxisLabels)
    {
        const std::string& key = kv.first;
        const std::string& label = kv.second;
        output << (firstLabel3D ? "" : "; ") << key << "=" << label;
        firstLabel3D = false;
    }
    output << std::endl;

    for(std::size_t iter = 0; iter < mData.size(); ++iter)
    {
        const data_point& point = *(mData[iter]);
        output << iter << "\t"
               << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << point(sdIndex) << "\t"
               << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << point(bpIndex) << "\t"
               << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << point(dlIndex) << std::endl;
    }
}

void data_export::writeImprovementCurvesFile(const std::string& fileName) const
{
    if(fileName == "")
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected a file name with at least one character, but the file name did not have any.\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if(mData.empty())
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected some data in mData to write an improvement-curves file, but mData is empty.\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if(mAxisDataTypes.size() < 3)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected at least three axis types (sd, bp, mADFbuilding_inverse) to write an improvement-curves file.\n"
                     << "Current axis types (" << mAxisDataTypes.size() << "): ";
        for(const auto& name : mAxisDataTypes) errorMessage << name << "; ";
        errorMessage << "\n(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }

    int sdIndex  = -1;
    int bpIndex  = -1;
    int dlIndex  = -1; // dl_ADFbuilding_inverse
    int dlScoreIndex = -1; // optional: dl_score (BS EN 17037-2018)
    for(std::size_t i = 0; i < mAxisDataTypes.size(); ++i)
    {
        const std::string& name = mAxisDataTypes[i];
        if(name == "sd") sdIndex = static_cast<int>(i);
        else if(name == "bp") bpIndex = static_cast<int>(i);
        else if(name == "dl_ADFbuilding_inverse") dlIndex = static_cast<int>(i);
        else if(name == "dl_score") dlScoreIndex = static_cast<int>(i);
    }
    if(sdIndex < 0 || bpIndex < 0 || dlIndex < 0)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, could not find required objective names 'sd', 'bp' and 'dl_ADFbuilding_inverse' in mAxisDataTypes for improvement curves.\n"
                     << "Current axis types: ";
        for(const auto& name : mAxisDataTypes) errorMessage << name << "; ";
        errorMessage << "\n(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }

    std::ofstream output(fileName.c_str());
    if(!output.is_open())
    {
        std::stringstream errorMessage;
        errorMessage << "Could not open the following file to write an improvement-curves data file for plotting to:" << std::endl
                     << fileName << std::endl
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }

    // Compute knee-point iteration using existing helper.
    std::vector<utilities::data_point> mDataNormalized;
    CombinedStats stats = computeCombinedStats(static_cast<unsigned int>(mData.size() - 1), mDataNormalized);

    // Initial values for percentage improvements.
    const data_point& first = *(mData[0]);
    const double sd0 = first(sdIndex);
    const double bp0 = first(bpIndex);
    const double dl0 = first(dlIndex);
    const bool hasDlScore = (dlScoreIndex >= 0);
    const double dlScore0 = hasDlScore ? first(dlScoreIndex) : 0.0;

    output << "# iteration\tsd_improvement_percent\tbp_improvement_percent\tdl_ADFbuilding_inverse_improvement_percent";
    if(hasDlScore)
    {
        output << "\tdl_score_improvement_percent";
    }
    output << std::endl;

    // Second header: knee-point iteration and axis labels from mAxisLabels
    // for the underlying objectives (sd, bp, dl_ADFbuilding_inverse, iteration).
    output << "# kneePointIteration " << stats.kneePointIteration;
    output << " axis_labels ";
    bool firstLabelImp = true;
    for(const auto& kv : mAxisLabels)
    {
        const std::string& key = kv.first;
        const std::string& label = kv.second;
        output << (firstLabelImp ? "" : "; ") << key << "=" << label;
        firstLabelImp = false;
    }
    output << std::endl;

    for(std::size_t iter = 0; iter < mData.size(); ++iter)
    {
        const data_point& point = *(mData[iter]);
        double sdVal = point(sdIndex);
        double bpVal = point(bpIndex);
        double dlVal = point(dlIndex);

        double sdImp = std::numeric_limits<double>::quiet_NaN();
        double bpImp = std::numeric_limits<double>::quiet_NaN();
        double dlImp = std::numeric_limits<double>::quiet_NaN();
        double dlScoreImp = std::numeric_limits<double>::quiet_NaN();

        if(sd0 != 0.0) sdImp = ((sd0-sdVal) / sd0) * 100.0;
        if(bp0 != 0.0) bpImp = ((bp0-bpVal) / bp0) * 100.0;
        if(dl0 != 0.0) dlImp = ((dl0-dlVal) / dl0) * 100.0;
        if(hasDlScore)
        {
            double dlScoreVal = point(dlScoreIndex);
            if(dlScore0 != 0.0)
            {
                dlScoreImp = ((dlScore0-dlScoreVal) / dlScore0) * 100.0;
            }
        }

        output << iter << "\t"
               << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdImp << "\t"
               << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpImp << "\t"
               << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << dlImp;
        if(hasDlScore)
        {
            output << "\t" << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << dlScoreImp;
        }
        output << std::endl;
    }
}

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
void data_export::initVectorToFile(const std::vector<T>& data, const std::vector<std::string>& dataTypes, const int& nrIterations, std::ofstream& output)
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
        output << iteration << ",," << std::endl;;
        //for(const auto type: dataTypes) output << ","; /*insert blanks for the places where no values where found*/
        //output << std::endl; // insert blanks for the places where no values where found
        iteration++;
    }
}

data_export::CombinedStats data_export::computeCombinedStats(const unsigned int nrIterations,
                                                             std::vector<utilities::data_point>& mDataNormalized) const
{
    CombinedStats stats;

    if(mData.empty())
    {
        return stats;
    }

    // Make a normalized copy for space iterations:
    for(const auto& i: mData)
    {
        mDataNormalized.push_back(*i);
    }
    utilities::data_point utopianPoint;
    utopianPoint.setToUtopia(mDataNormalized);
    utilities::data_point distopianPoint;
    distopianPoint.setToDistopia(mDataNormalized);
    for (auto& j : mDataNormalized)
    {
        j.normalize(utopianPoint,distopianPoint,{1});
    }
    utopianPoint.setToUtopia(mDataNormalized);
    distopianPoint.setToDistopia(mDataNormalized);

    // Determine the Best of each dataPoint and the KneePoint design
    stats.kneePointIteration = 0;
    const unsigned int nObjectives = (*mData[0]).size();
    stats.bestIterations.assign(nObjectives, 0);
    stats.bestDataPoints.assign(nObjectives, *(mData[0]));
    stats.bestDataPointsNormalized.assign(nObjectives, mDataNormalized[0]);
    stats.bestImprovementPercentages.assign(nObjectives, 0.0);
    stats.lastImprovementPercentages.assign(nObjectives, 0.0);
    std::vector<data_point> kneePointVectorNor;

    // Knee-point objective: all objectives are treated as "to be minimized",
    // so the utopian point in normalized space is the zero vector.
    utilities::data_point kneePointObjective(nObjectives);
    kneePointVectorNor = kneePointObjective.findClosestIn(mDataNormalized);
    for(long unsigned int i=0; i < mData.size(); i++)
    {
        for(unsigned int obj = 0; obj < nObjectives; ++obj)
        {
            // All objectives are minimized: smaller normalized value is better.
            if(mDataNormalized[i](obj) < stats.bestDataPointsNormalized[obj](obj))
            {
                stats.bestIterations[obj] = static_cast<int>(i);
                stats.bestDataPoints[obj] = *(mData[i]);
                stats.bestDataPointsNormalized[obj] = mDataNormalized[i];
            }
        }

        if(kneePointVectorNor[0] == mDataNormalized[i])
        {
            stats.kneePoint = *(mData[i]);
            stats.kneePointIteration = i;
        }
    }

    // Compute per-objective improvement percentages relative to the first design
    const unsigned int lastIndex = static_cast<unsigned int>(mData.size() - 1);
    for(unsigned int obj = 0; obj < nObjectives; ++obj)
    {
        const double initialValue = (*(mData[0]))(obj);
        if(initialValue != 0.0)
        {
            const double bestValue = (*(mData[stats.bestIterations[obj]]))(obj);
            const double lastValue = (*(mData[lastIndex]))(obj);
            stats.bestImprovementPercentages[obj] = ((initialValue-bestValue)/initialValue)*100.0;
            stats.lastImprovementPercentages[obj] = ((initialValue-lastValue)/initialValue)*100.0;
        }
        else
        {
            // Use NaN to indicate that the percentage is undefined
            const double invalid = std::numeric_limits<double>::quiet_NaN();
            stats.bestImprovementPercentages[obj] = invalid;
            stats.lastImprovementPercentages[obj] = invalid;
        }
    }

    return stats;
}

data_export::MsDesignData data_export::collectMsDesignData(const std::vector<bso::spatial_design::ms_N_building>& msDesigns) const
{
    MsDesignData msData;

    // Temporary storage for the data
    double medianTemp;
    double firstQuartileTemp;
    double thirdQuartileTemp;
    double averageTemp;
    std::vector<double> outliersTemp;

    for(auto& design: msDesigns)
    {
        msData.averageRecDif.push_back(design.getAverageRecDif());
        msData.ratioBiggestSmallestFloorAreaSpace.push_back(design.getSpaceFloorAreaRatio());
        msData.nrElevations.push_back(design.getNrElevations());
        design.getSpaceFloorAreaBoxPlotValues(medianTemp, firstQuartileTemp, thirdQuartileTemp, averageTemp, outliersTemp);
        msData.median.push_back(medianTemp);
        msData.firstQuartile.push_back(firstQuartileTemp);
        msData.thirdQuartile.push_back(thirdQuartileTemp);
        msData.average.push_back(averageTemp);
        std::vector<bso::spatial_design::ms_space*> spacesNonBiggestFragment;
        msData.isFragmented.push_back(design.isFragmented(spacesNonBiggestFragment));
        // Note: outliers are currently not written to file, so we do not store them
    }

    return msData;
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
    if(mData.size() == 0)
    {
        if((*mData[0]).size() != mAxisDataTypes.size())
        {
            std::stringstream errorMessage;
                    errorMessage << "\nError, expected the size of the data points in mData to be equal to the size of mAxisDataTypes \n"
                                            << "to know which data is which and to give the plot axis variables.\n"
                                            << "The found size of the data points in mData is: " << (*mData[0]).size() 
                                            << "\nand it contains these variables: " << std::endl;
                                            for(const auto& i: mAxisDataTypes) errorMessage << i << "; ";
                                            errorMessage << "(bso/utilities/data_export.cpp). " << std::endl;
            throw std::invalid_argument(errorMessage.str());
        }
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
    std::vector<utilities::data_point> mDataNormalized;
    CombinedStats stats = computeCombinedStats(nrIterations, mDataNormalized);

    // Generate output data where the index is equal to the considered iteration in msDesigns
    MsDesignData msData = collectMsDesignData(msDesigns);

    // output the data to a file
    if (!inputFile.is_open())
    {
        // File does not exist yet, so initialize with headers and first data values:
        outputFile << "," << designName << "_" << runKind;
        for(int axisIndex = 1; axisIndex < mAxisDataTypes.size(); axisIndex++)
        {
            outputFile << ",";
        }
        outputFile << "," << std::endl << ",";
        for(std::string i: mAxisDataTypes)
        {
            outputFile << i << ",";
        }
        outputFile << std::endl;
        unsigned int iteration = 0;

        // Generate data and empty line if nr. of iteration was not reached
        for(const auto& i: mData)
        {
            outputFile << iteration << ",";
            for(unsigned int j=0; j< mAxisDataTypes.size(); j++)
            {
                outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*i)(j) << ",";
            }
            outputFile << std::endl;
            iteration++;
        }
        while(iteration <= nrIterations)
        {
            outputFile << iteration << ",";
            for(unsigned int j=0; j< mAxisDataTypes.size(); j++)
            {
                outputFile << ","; // insert blanks for the places where no values where found
            }
            outputFile << std::endl;
            iteration++;
        }
        outputFile << "kneePoint,";
        if(mData.size() == 0){for(long unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}}
        else
        {
            for(long unsigned int j=0; j< stats.kneePoint.size(); j++){outputFile << stats.kneePoint(j) << ",";}
        }
        outputFile << std::endl;

        // Output a "<axisName>Best" row for each objective, using the
        // corresponding bestDataPoints entry when available.
        for(long unsigned int obj = 0; obj < mAxisDataTypes.size(); ++obj)
        {
            outputFile << mAxisDataTypes[obj] << "Best,";
            if(mData.size() == 0)
            {
                for(long unsigned int j = 0; j < mAxisDataTypes.size(); ++j){outputFile << ",";}
            }
            else if(obj < stats.bestDataPoints.size())
            {
                for(unsigned int j = 0; j < stats.bestDataPoints[obj].size(); ++j){outputFile << stats.bestDataPoints[obj](j) << ",";}
            }
            else
            {
                for(long unsigned int j = 0; j < mAxisDataTypes.size(); ++j){outputFile << ",";}
            }
            outputFile << std::endl;
        }
        if(mData.size() == 0) // output empty so set spacing for correct csv file
        {
            outputFile << "kneePoint iteration,";
            for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
            outputFile << std::endl;
            for(const auto objectiveName: mAxisDataTypes)
            {
                outputFile << objectiveName << "Best iteration,";
                for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
                outputFile<< std::endl;
            }
            for(const auto objectiveName: mAxisDataTypes)
            {
                outputFile << objectiveName << " last iteration vs. first iteration,";
                for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
                outputFile<< std::endl;
            }
            for(const auto objectiveName: mAxisDataTypes)
            {
                outputFile << objectiveName << " best iteration vs. first iteration,";
                for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
                outputFile<< std::endl;
            }

            // output the msDesign data:
            outputFile << "," << designName << runKind << std::endl;
            outputFile << "MS Design data,Recdif" << std::endl;
            std::vector<double> emptyDataVector;
            initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
            outputFile << ",Ratio biggest vs. smallest floor area space" << std::endl;
            initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
            outputFile << ",nrElevations" << std::endl;
            initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
            outputFile << ",total loads" << std::endl;
            initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
            outputFile << ",exterior surface area," << std::endl;
            initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
            outputFile << ",isFragmented" << std::endl;
            initVectorToFile(emptyDataVector, mAxisDataTypes, nrIterations, outputFile);
        }
        else
        {
            outputFile << "kneePoint iteration," << stats.kneePointIteration;;
            for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
            outputFile << std::endl;
            for(unsigned int index=0; index<mAxisDataTypes.size(); index++)
            {
                outputFile << mAxisDataTypes[index] << "Best iteration," << stats.bestIterations[index];
                for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
                outputFile << std::endl;
            }
            for(unsigned int index=0; index<mAxisDataTypes.size(); index++)
            {
                outputFile << mAxisDataTypes[index] << " last iteration vs. first iteration," << stats.lastImprovementPercentages[index];
                for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
                outputFile << std::endl;
            }
            for(unsigned int index=0; index<mAxisDataTypes.size(); index++)
            {
                outputFile << mAxisDataTypes[index] << " best iteration vs. first iteration," << stats.bestImprovementPercentages[index];
                for(unsigned int j=0; j< mAxisDataTypes.size(); j++){outputFile << ",";}
                outputFile << std::endl;
            }

            // output the msDesign data:
            outputFile << "\n," << designName << runKind << std::endl;
            outputFile << "MS Design data,Recdif" << std::endl;
            initVectorToFile(msData.averageRecDif, mAxisDataTypes, nrIterations, outputFile);
            outputFile << ",Ratio biggest vs. smallest floor area space" << std::endl;
            initVectorToFile(msData.ratioBiggestSmallestFloorAreaSpace, mAxisDataTypes, nrIterations, outputFile);
            outputFile << ",nrElevations" << std::endl;
            initVectorToFile(msData.nrElevations, mAxisDataTypes, nrIterations, outputFile);
            outputFile << ",total loads" << std::endl;
            initVectorToFile(sd_total_load, mAxisDataTypes, nrIterations, outputFile);
            outputFile << ",exterior surface area" << std::endl;
            initVectorToFile(exteriorWallArea, mAxisDataTypes, nrIterations, outputFile);
            outputFile << ",isFragmented" << std::endl;
            initVectorToFile(msData.isFragmented, mAxisDataTypes, nrIterations, outputFile);
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
            if(lineCount == 0)
            {
                line += designName + runKind + ",";
                for(int axisIndex = 1; axisIndex < mAxisDataTypes.size(); axisIndex++){line += ",";}
            }
            else if(lineCount == 1)
            {
                for(std::string i: mAxisDataTypes)
                {
                    line +=  i  + ",";
                }
            }
            else if(dataCount < mData.size())
            {
                for(int j=0; j< mAxisDataTypes.size(); j++)
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
                for(long unsigned int j=0; j< mAxisDataTypes.size(); j++)
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
                    for(long unsigned int j=0; j< mAxisDataTypes.size(); j++){oss << ",";}
                    lineCount++;
                }
                line += oss.str();
            }
            else
            {
                std::ostringstream oss;
                const unsigned int nObjectives = mAxisDataTypes.size();

                // Line indices for summary block (knee point and per-objective stats)
                const unsigned int kneeLine           = dataCount + 2;
                const unsigned int firstBestLine      = kneeLine + 1;                 // one line per objective
                const unsigned int kneeIterLine       = firstBestLine + nObjectives;   // knee point iteration
                const unsigned int firstBestIterLine  = kneeIterLine + 1;              // one line per objective
                const unsigned int firstLastImpLine   = firstBestIterLine + nObjectives; // one line per objective
                const unsigned int firstBestImpLine   = firstLastImpLine + nObjectives;  // one line per objective
                const unsigned int emptyLineIndex     = firstBestImpLine + nObjectives;  // empty separator line
                const unsigned int designNameLine     = emptyLineIndex + 1;             // design name line
                const unsigned int recdifHeaderLine   = designNameLine + 1;             // "MS Design data,Recdif" line

                if (lineCount == kneeLine)
                {
                    // kneePoint row
                    dataPointToStream(stats.kneePoint, oss);
                }
                else if (lineCount >= firstBestLine && lineCount < firstBestLine + nObjectives)
                {
                    // "<axisName>Best" rows: one per objective
                    unsigned int obj = lineCount - firstBestLine;
                    if (obj < stats.bestDataPoints.size())
                    {
                        dataPointToStream(stats.bestDataPoints[obj], oss);
                    }
                    else
                    {
                        for (unsigned int j = 0; j < nObjectives; ++j) oss << ","; // blanks when no data
                    }
                }
                else if (lineCount == kneeIterLine)
                {
                    // kneePoint iteration row
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10)
                        << stats.kneePointIteration << ",";
                    for (unsigned int j = 1; j < nObjectives; ++j) oss << ",";
                }
                else if (lineCount >= firstBestIterLine && lineCount < firstBestIterLine + nObjectives)
                {
                    // "<axisName>Best iteration" rows
                    unsigned int obj = lineCount - firstBestIterLine;
                    int value = (obj < stats.bestIterations.size()) ? stats.bestIterations[obj] : 0;
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10)
                        << value << ",";
                    for (unsigned int j = 1; j < nObjectives; ++j) oss << ",";
                }
                else if (lineCount >= firstLastImpLine && lineCount < firstLastImpLine + nObjectives)
                {
                    // "<axisName> last iteration vs. first iteration" rows
                    unsigned int obj = lineCount - firstLastImpLine;
                    double value = (obj < stats.lastImprovementPercentages.size()) ? stats.lastImprovementPercentages[obj] : 0.0;
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10)
                        << value << ",";
                    for (unsigned int j = 1; j < nObjectives; ++j) oss << ",";
                }
                else if (lineCount >= firstBestImpLine && lineCount < firstBestImpLine + nObjectives)
                {
                    // "<axisName> best iteration vs. first iteration" rows
                    unsigned int obj = lineCount - firstBestImpLine;
                    double value = (obj < stats.bestImprovementPercentages.size()) ? stats.bestImprovementPercentages[obj] : 0.0;
                    oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10)
                        << value << ",";
                    for (unsigned int j = 1; j < nObjectives; ++j) oss << ",";
                }
                else if (lineCount == emptyLineIndex)
                {
                    // separator line between summary block and MS design data: leave empty
                }
                else if (lineCount == designNameLine)
                {
                    oss << "," << designName << runKind;
                }
                else if (lineCount == recdifHeaderLine)
                {
                    oss << ",Recdif";
                }
                // MS design data blocks (Recdif and following metrics)
                else if (recdifHeaderLine + 1 + nrIterations >= lineCount)
                {
                    vectorToStream(msData.averageRecDif, iteration, nrIterations,
                                   msDesigns.size() + recdifHeaderLine + 1,
                                   lineCount, oss);
                }
                else if (recdifHeaderLine + 2 + nrIterations == lineCount)
                {
                    oss << ",Ratio biggest vs. smallest floor area space"; iteration = 0;
                }
                else if (recdifHeaderLine + 3 + nrIterations * 2 >= lineCount)
                {
                    vectorToStream(msData.ratioBiggestSmallestFloorAreaSpace, iteration, nrIterations,
                                   msDesigns.size() + recdifHeaderLine + 3 + nrIterations,
                                   lineCount, oss);
                }
                else if (recdifHeaderLine + 4 + nrIterations * 2 == lineCount)
                {
                    oss << ",nrElevations"; iteration = 0;
                }
                else if (recdifHeaderLine + 5 + nrIterations * 3 >= lineCount)
                {
                    vectorToStream(msData.nrElevations, iteration, nrIterations,
                                   msDesigns.size() + recdifHeaderLine + 5 + nrIterations * 2,
                                   lineCount, oss);
                }
                else if (recdifHeaderLine + 6 + nrIterations * 3 == lineCount)
                {
                    oss << ",total loads"; iteration = 0;
                }
                else if (recdifHeaderLine + 7 + nrIterations * 4 >= lineCount)
                {
                    vectorToStream(sd_total_load, iteration, nrIterations,
                                   msDesigns.size() + recdifHeaderLine + 7 + nrIterations * 3,
                                   lineCount, oss);
                }
                else if (recdifHeaderLine + 8 + nrIterations * 4 == lineCount)
                {
                    oss << ",exterior surface area"; iteration = 0;
                }
                else if (recdifHeaderLine + 9 + nrIterations * 5 >= lineCount)
                {
                    vectorToStream(exteriorWallArea, iteration, nrIterations,
                                   msDesigns.size() + recdifHeaderLine + 9 + nrIterations * 4,
                                   lineCount, oss);
                }
                else if (recdifHeaderLine + 10 + nrIterations * 5 == lineCount)
                {
                    oss << ",isFragmented"; iteration = 0;
                }
                else if (recdifHeaderLine + 11 + nrIterations * 6 >= lineCount)
                {
                    vectorToStream(msData.isFragmented, iteration, nrIterations,
                                   msDesigns.size() + recdifHeaderLine + 11 + nrIterations * 5,
                                   lineCount, oss);
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

// void data_export::addToOverviewCombinedDataFile(const std::string& fileName, const std::string& runKind, const std::string& designName, const unsigned int numberOfIterationsPerformed, const bso::spatial_design::ms_N_building& initialDesign)
// { 
//     if(fileName == "")
//     {
//         std::stringstream errorMessage;
// 				errorMessage << "\nError, expected a file name with at least one character, but the file name did not had any.\n"
//                                          << "(bso/utilities/data_export.cpp). " << std::endl;
// 				throw std::invalid_argument(errorMessage.str());
//     }
//     if(designName == "")
//     {
//         std::stringstream errorMessage;
// 				errorMessage << "\nError, expected a designName with at least one character, but the designName name did not had any.\n"
//                                          << "(bso/utilities/data_export.cpp). " << std::endl;
// 				throw std::invalid_argument(errorMessage.str());
//     }
//     if(runKind == "")
//     {
//         std::stringstream errorMessage;
// 				errorMessage << "\nError, expected a runKind with at least one character, but the runKind name did not had any.\n"
//                                          << "(bso/utilities/data_export.cpp). " << std::endl;
// 				throw std::invalid_argument(errorMessage.str());
//     }

//     // Check ouput file:
//     std::ifstream inputFile(fileName);
//     std::ofstream outputFile("temp.txt");
//     if (!outputFile.is_open())
//     { 
//         std::stringstream errorMessage;
// 				errorMessage << "\nError, unable to open file.\n"
//                                          << "(bso/utilities/data_export.cpp). " << std::endl;
// 				throw std::invalid_argument(errorMessage.str());
//     }

//     // Get relevant data to write to file:
//     // Make a normalized copy for space iterations:

//     // // Get relevant data to write to file:
//     // std::vector<utilities::data_point> mDataNormalized;
//     // CombinedStats stats = computeCombinedStats(nrIterations, mDataNormalized);

//     // // Generate output data where the index is equal to the considered iteration in msDesigns
//     // MsDesignData msData = collectMsDesignData(msDesigns);


//     std::vector<utilities::data_point> mDataNormalized;
//     for(const auto& i: mData)
//     {
//         mDataNormalized.push_back(*i);
//     }
//     utilities::data_point utopianPoint;
// 	utopianPoint.setToUtopia(mDataNormalized);
//     utilities::data_point distopianPoint;
// 	distopianPoint.setToDistopia(mDataNormalized);
//     std::swap(utopianPoint(1),distopianPoint(1));
//     for (auto& j : mDataNormalized)
//     {
//         j.normalize(utopianPoint,distopianPoint,{1});
//     }
//     utopianPoint.setToUtopia(mDataNormalized);
// 	distopianPoint.setToDistopia(mDataNormalized);

//     // Determine the sd Best, bp Best and kneePoint design
//     int kneePointIteration = 0;
//     int sdBestIteration = 0;
//     int bpBestIteration = 0;
//     utilities::data_point sdBest = *(mData[0]);
//     utilities::data_point sdBestNormalized = mDataNormalized[0];
//     utilities::data_point bpBest = *(mData[0]);
//     utilities::data_point bpBestNormalized = mDataNormalized[0];
//     utilities::data_point kneePoint;
//     utilities::data_point kneePointObjective({0,1});
//     std::vector<data_point> kneePointVectorNor = kneePointObjective.findClosestIn(mDataNormalized);
//     for(long unsigned int i=0; i < mData.size(); i++)
//     {
//         if(mDataNormalized[i](0) < sdBestNormalized(0))
//         { // determine the best design in SD
//             sdBest = *(mData[i]);
//             sdBestNormalized = mDataNormalized[i];
//             sdBestIteration = i;
//         }
//         if(mDataNormalized[i](1) > bpBestNormalized(1))
//         { // determine the best design in BP
//             bpBest = *(mData[i]);
//             bpBestNormalized = mDataNormalized[i];
//             bpBestIteration = i;
//         }
//         if(kneePointVectorNor[0] == mDataNormalized[i])
//         {
//             kneePoint = *(mData[i]);
//             kneePointIteration = i;
//         }
//     }

//     double SDBestImprovementPercentage = (1-((*(mData[sdBestIteration]))(0)/(*(mData[0]))(0)))*100;
//     double BPBestImprovementPercentage = (1-((*(mData[bpBestIteration]))(1)/(*(mData[0]))(1)))*100;
//     double SDLastImprovementPercentage = (1-((*(mData[mData.size()-1]))(0)/(*(mData[0]))(0)))*100;
//     double BPLastImprovementPercentage = (1-((*(mData[mData.size()-1]))(1)/(*(mData[0]))(1)))*100;
    
//     if (!inputFile.is_open())
//     {
//         // initiate descriptive line:
//         // initial design data:
//         outputFile << ",Nr spaces,Floor area,Nr.Elevation,Ratio biggest vs. smallest space,RecDif,";

//         // run data
//         outputFile << "best sd design in sd run";
//         for(std::string i: mAxisDataTypes) { outputFile << ",";}
//         outputFile << "best bp design in sd run";
//         for(std::string i: mAxisDataTypes) { outputFile << ",";}
//         outputFile << "Last design of sd run";
//         for(std::string i: mAxisDataTypes) { outputFile << ",";}
//         for(int j=0; j < 7; j++) outputFile << ",";

//         outputFile << "best bp design in bp run";
//         for(std::string i: mAxisDataTypes) { outputFile << ",";}
//         outputFile << "best sd design in bp run";
//         for(std::string i: mAxisDataTypes) { outputFile << ",";}
//         outputFile << "Last design of bp run";
//         for(std::string i: mAxisDataTypes) { outputFile << ",";}
//         for(int j=0; j < 7; j++) outputFile << ",";

//         outputFile << "best sd design in weighted run";
//         for(std::string i: mAxisDataTypes) { outputFile << ",";}
//         outputFile << "best bp design in weighted run";
//         for(std::string i: mAxisDataTypes) { outputFile << ",";}
//         outputFile << "Last design of weighted run";
//         for(std::string i: mAxisDataTypes) { outputFile << ",";}
//         outputFile << std::endl;
        
//         // make rule with value indicators:
//         for(int i=0; i<6; i++) {outputFile << ",";}
//         for(int j=0; j < 3; j++)
//         {
//             for(std::string i: mAxisDataTypes)
//             {
//                 outputFile <<  i  + ",";
//             }
//         }
//         outputFile << "sd best iteration,bp best iteration,sd improvement last vs. initial,bp improvement last vs. initial,sd improvement best vs. initial,bp improvement best vs. initial,number Of iteration Performed,";
//         for(int j=0; j < 3; j++)
//         {
//             for(std::string i: mAxisDataTypes)
//             {
//                 outputFile <<  i  + ",";
//             }
//         }
//         outputFile << "sd best iteration,bp best iteration,sd improvement last vs. initial,bp improvement last vs. initial,sd improvement best vs. initial,bp improvement best vs. initial,number Of iteration Performed,";
//         for(int j=0; j < 3; j++)
//         {
//             for(std::string i: mAxisDataTypes)
//             {
//                 outputFile <<  i  + ",";
//             }
//         }
//         outputFile << "sd best iteration,bp best iteration,sd improvement last vs. initial,bp improvement last vs. initial,sd improvement best vs. initial,bp improvement best vs. initial,number Of iteration Performed,";
//         outputFile << std::endl;

//         // output performance rule of data:
//         if(mData.size() == 0)
//         {
//             // output enough ',' to fill the space of the run
//             outputFile << designName << ",";
//             outputFile << initialDesign.getSpacePtrs().size() << ",";
//             outputFile << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getFloorArea() << ",";
//             outputFile << initialDesign.getNrElevations() << ",";
//             outputFile << initialDesign.getSpaceFloorAreaRatio() << ",";
//             outputFile << initialDesign.getAverageRecDif() << ",";
//             for(long unsigned int j=0; j < mAxisDataTypes.size(); j++){outputFile << ",";} 
//             for(long unsigned int j=0; j < mAxisDataTypes.size(); j++){outputFile << ",";} 
//             for(long unsigned int j=0; j < mAxisDataTypes.size(); j++){outputFile << ",";}
//             for(long unsigned int j=0; j < 7; j++) outputFile << ",";
//         }
//         else
//         {
//             int lastIndex =  mData.size()-1;
//             outputFile << designName << ",";
//             outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getSpacePtrs().size() << ",";
//             outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getFloorArea() << ",";
//             outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getNrElevations() << ",";
//             outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getSpaceFloorAreaRatio() << ",";
//             outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getAverageRecDif() << ",";
//             for(unsigned int j=0; j < sdBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBest(j) << ",";} 
//             for(unsigned int j=0; j < bpBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBest(j) << ",";} 
//             for(unsigned int j=0; j < (*(mData[lastIndex])).size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*(mData[lastIndex]))(j) << ",";} // performance of last design of run
//             outputFile << sdBestIteration << "," ;
//             outputFile << bpBestIteration << "," ;
//             outputFile << SDLastImprovementPercentage << "," ;
//             outputFile << BPLastImprovementPercentage << "," ;
//             outputFile << SDBestImprovementPercentage << "," ;
//             outputFile << BPBestImprovementPercentage << "," ;
//             outputFile << numberOfIterationsPerformed << "," ;
//         }

//         inputFile.close();
//         outputFile.close();
//     } 
//     else 
//     {
//         // Is there already a line for the input design
//         int lineCount = 0;
//         std::string line;
//         bool lineFound = false;
//         while (getline(inputFile, line)) 
//         {
//             if(lineCount == 0 || lineCount == 1)
//             {
//                 outputFile << line << std::endl;
//                 lineCount++; 
//                 continue;
//             }
//             if(designName == line.substr(0,line.find(",")))
//             { 
//                 // output performance rule of data at the end of the design data line:
//                 std::ostringstream oss;
//                 if(mData.size() == 0)
//                 {
//                     // output enough ',' to fill the space of the run
//                     for(long unsigned int j=0; j < mAxisDataTypes.size(); j++){outputFile << ",";} 
//                     for(long unsigned int j=0; j < mAxisDataTypes.size(); j++){outputFile << ",";} 
//                     for(long unsigned int j=0; j < mAxisDataTypes.size(); j++){outputFile << ",";}
//                     for(long unsigned int j=0; j < 7; j++) outputFile << ",";
//                 }
//                 else
//                 {
//                     int lastIndex =  mData.size()-1;
//                     for(unsigned int j=0; j < sdBest.size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBest(j) << ",";} 
//                     for(unsigned int j=0; j < bpBest.size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBest(j) << ",";} 
//                     for(unsigned int j=0; j < (*(mData[lastIndex])).size(); j++){oss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*(mData[lastIndex]))(j) << ",";} // performance of last design of run
//                     oss << sdBestIteration << "," ;
//                     oss << bpBestIteration << "," ;
//                     oss << SDLastImprovementPercentage << "," ;
//                     oss << BPLastImprovementPercentage << "," ;
//                     oss << SDBestImprovementPercentage << "," ;
//                     oss << BPBestImprovementPercentage << "," ;
//                     oss << numberOfIterationsPerformed << "," ;
//                 }
                
//                 lineFound = true;

//                 // Write the modified line to the temporary file
//                 line += oss.str();
//                 outputFile << line << std::endl;
//                 break;
//             }
//             else
//             {
//                 outputFile << line << std::endl;
//             }
//             lineCount++;
//         }

//         if(!lineFound)
//         {
//             // output performance rule of data:
//             if(mData.size() == 0)
//             {
//                 // output enough ',' to fill the space of the run
//                 outputFile << designName << ",";
//                 outputFile << initialDesign.getSpacePtrs().size() << ",";
//                 outputFile << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getFloorArea() << ",";
//                 outputFile << initialDesign.getNrElevations() << ",";
//                 outputFile << initialDesign.getSpaceFloorAreaRatio() << ",";
//                 outputFile << initialDesign.getAverageRecDif() << ",";
//                 for(long unsigned int j=0; j < mAxisDataTypes.size(); j++){outputFile << ",";} 
//                 for(long unsigned int j=0; j < mAxisDataTypes.size(); j++){outputFile << ",";} 
//                 for(long unsigned int j=0; j < mAxisDataTypes.size(); j++){outputFile << ",";}
//                 for(int j=0; j < 7; j++) outputFile << ",";
//             }
//             else
//             {
//                 int lastIndex =  mData.size()-1;
//                 outputFile << designName << ",";
//                 outputFile << initialDesign.getSpacePtrs().size() << ",";
//                 outputFile << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getFloorArea() << ",";
//                 outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getNrElevations() << ",";
//                 outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getSpaceFloorAreaRatio() << ",";
//                 outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << initialDesign.getAverageRecDif() << ",";
//                 for(unsigned int j=0; j < sdBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << sdBest(j) << ",";} 
//                 for(unsigned int j=0; j < bpBest.size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << bpBest(j) << ",";} 
//                 for(unsigned int j=0; j < (*(mData[lastIndex])).size(); j++){outputFile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << (*(mData[lastIndex]))(j) << ",";} // performance of last design of run
//                 outputFile << sdBestIteration << "," ;
//                 outputFile << bpBestIteration << "," ;
//                 outputFile << SDLastImprovementPercentage << "," ;
//                 outputFile << BPLastImprovementPercentage << "," ;
//                 outputFile << SDBestImprovementPercentage << "," ;
//                 outputFile << BPBestImprovementPercentage << "," ;
//                 outputFile << numberOfIterationsPerformed << "," ;
//             }
//         }
    
//         inputFile.close();
//         outputFile.close();

//         // Remove original file
//         if (std::remove(fileName.c_str()) != 0) 
//         {
//             std::stringstream errorMessage;
//                 errorMessage << "\nError, while deleting original file.\n"
//                                         << "(bso/utilities/data_export.cpp). " << std::endl;
//                 throw std::invalid_argument(errorMessage.str());
//         } 
//     }
    
//     // Rename the temporary file to the original file name
//     if (std::rename("temp.txt", fileName.c_str()) != 0) 
//     {
//         std::stringstream errorMessage;
//             errorMessage << "\nError, renaming temporary file.\n"
//                                     << "(bso/utilities/data_export.cpp). " << std::endl;
//             throw std::invalid_argument(errorMessage.str());
//     } 
//     else 
//     {            
//         std::cout << "In addToOverviewCombinedDataFile: File processing complete and temp file successfully renamed." << std::endl;
//     }
    
// } // addToOverviewCombinedDataFile()

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
    if(mAxisDataTypes.size() != 2)
    {
         std::stringstream errorMessage;
				errorMessage << "\nError, expected two axis types to be defined to know which data is which and to give the plot axis variables.\n"
										 << "The current amount of mAxisDataTypes is: " << mAxisDataTypes.size() << " and it contains these variables: " << std::endl;
                                         for(const auto& i: mAxisDataTypes) errorMessage << i << "; ";
                                         errorMessage << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Generate the mScatterData file if none is currently already existing
    if(mScatterData == "")
    {
        this->writeDataToFile(mData, fileNameScatter,"scatterData");
    }   

    // Define part of the a terminal command to make a plot with GNUplot
    std::string plotDetails = "plot '" + fileNameScatter  + "' using 1:2 title '" + mDataName + "'\"";
    std::string axisSetting = "set xlabel '" + mAxisDataTypes[0] + "'; set ylabel '" + mAxisDataTypes[1] + "';";

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
    if(mAxisDataTypes.size() != 2)
    {
         std::stringstream errorMessage;
				errorMessage << "\nError, expected two axis types to be defined to know which data is which and to give the plot axis variables.\n"
										 << "The current amount of mAxisDataTypes is: " << mAxisDataTypes.size() << " and it contains these variables: " << std::endl;
                                         for(const auto& i: mAxisDataTypes) errorMessage << i << "; ";
                                         errorMessage << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    
    if(mParetoData == "" || mNonParetoData == "")
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
    std::string plotDetails = "plot '" + fileNameNonPareto  + "' using 1:2 title '" + mDataName + "', '" + fileNamePareto + "' using 1:2 title '" + mDataName + " on pareto'\"";
    std::string axisSetting = "set xlabel '" + mAxisDataTypes[0] + "'; set ylabel '" + mAxisDataTypes[1] + "';";

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
    if(mAxisDataTypes.size() != 2)
    {
         std::stringstream errorMessage;
				errorMessage << "\nError, expected two axis types to be defined to know which data is which and to give the plot axis variables.\n"
										 << "The current amount of mAxisDataTypes is: " << mAxisDataTypes.size() << " and it contains these variables: " << std::endl;
                                         for(const auto& i: mAxisDataTypes) errorMessage << i << "; ";
                                         errorMessage << "(bso/utilities/data_export.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Make a data file with four columns where the first datapoint (x,y) needs to connect to the next two datapoint on the line (xdelta,ydelta) link:http://gnuplot.info/docs_6.0/loc6150.html
    if(mVectorStyleData == "")
    {
        this->writeVectorStyleFile(mData, fileNameVector);
    }
    
    // Send a terminal command to make a plot with GNUplot
    std::string plotDetails = "plot '" + fileNameVector  + "' using 1:2:3:4 with vectors filled head title '" + mDataName + " progression'\"";
    std::string axisSetting = "set xlabel '" + mAxisDataTypes[0] + "'; set ylabel '" + mAxisDataTypes[1] + "';";

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

void data_export::plotMultiAxis2D(std::string fileNameMultiAxis /*= "multiAxisData"*/, std::string pngName /*= "plotMultiAxis.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/)
{
    if(fileNameMultiAxis == "")
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
    if(mAxisDataTypes.size() <= 1)
    {
         std::stringstream errorMessage;
                errorMessage << "\nError, expected at least one or multiple axis types to be defined to know which data is which and to give the plot axis variables.\n"
                                         << "The current amount of mAxisDataTypes is: " << mAxisDataTypes.size() << " and it contains these variables: " << std::endl;
                                         for(const auto& i: mAxisDataTypes) errorMessage << i << "; ";
                                         errorMessage << "(bso/utilities/data_export.cpp). " << std::endl;
                throw std::invalid_argument(errorMessage.str());
    }

    // Generate the mMultiAxisData file if none is currently already existing
    if(mMultiAxisData == "")
    {
        this->writeMultiAxisFile(mData, fileNameMultiAxis);
    }

    // Use external Python script (Matplotlib) to generate the multi-axis plot.
    // The script is located in bso/utilities/plot/plot_multi_axis.py and expects:
    //   1) path to the data file
    //   2) optional plot title
    //   3) show flag  ("1" = show window, "0" = no window)
    //   4) save flag  ("1" = save PNG,   "0" = do not save)
    //   5) png path   (optional explicit PNG output path)

    std::string scriptPath = "/home/tessa/BSO-toolbox-V2/bso/utilities/plot/plot_multi_axis.py";

    // Build command:
    //   [MPLBACKEND=Agg] python3 <script> <dataFile> "<title>" <show> <save> "<pngName>"
    // When visualize == false, force a non-GUI backend so no X11 / DISPLAY is required.
    std::stringstream cmd;
    if(!visualize)
    {
        cmd << "MPLBACKEND=Agg ";
    }

    cmd << "python3 "
        << scriptPath << " "
        << fileNameMultiAxis << " \"" << mDataName << "\" "
        << (visualize ? "1" : "0") << " "
        << (writeToPNG ? "1" : "0") << " \"" << pngName << "\"";

    int systemRet = system(cmd.str().c_str());
    if(systemRet == -1)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, the plotting of the multi-axis graph did not go as expected and an error has been returned.\n"
                     << "The following fileName was used for plotting: " << fileNameMultiAxis << "\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
} // plotMultiAxis2D()

void data_export::plotObjectiveSpace3D(std::string fileNameObjective3D /*= "objective3DData"*/, std::string pngName /*= "plotObjective3D.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/)
{
    if(fileNameObjective3D == "")
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected a file name with at least one character for the 3D objective plot, but the file name did not have any.\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if(!visualize && !writeToPNG)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, no 3D objective plot will be generated since both booleans 'visualize' and 'writeToPNG' are false.\n"
                     << "Make sure either one or both are set to true when this function is called to get results.\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }

    // Generate the 3D objective data file (sd, bp, mADFbuilding_inverse + knee metadata)
    this->writeObjective3DFile(fileNameObjective3D);

    std::string scriptPath = "/home/tessa/BSO-toolbox-V2/bso/utilities/plot/plot_objective_space_3d.py";

    std::stringstream cmd;
    if(!visualize)
    {
        cmd << "MPLBACKEND=Agg ";
    }

    cmd << "python3 "
        << scriptPath << " "
        << fileNameObjective3D << " \"" << mDataName << "\" "
        << (visualize ? "1" : "0") << " "
        << (writeToPNG ? "1" : "0") << " \"" << pngName << "\"";

    int systemRet = system(cmd.str().c_str());
    if(systemRet == -1)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, the plotting of the 3D objective graph did not go as expected and an error has been returned.\n"
                     << "The following fileName was used for plotting: " << fileNameObjective3D << "\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
} // plotObjectiveSpace3D()

void data_export::plotImprovementCurves(std::string fileNameImprovement /*= "improvementCurvesData"*/, std::string pngName /*= "plotImprovementCurves.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/, bool showKnee /*= false*/)
{
    if(fileNameImprovement == "")
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, expected a file name with at least one character for the improvement-curves plot, but the file name did not have any.\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
    if(!visualize && !writeToPNG)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, no improvement-curves plot will be generated since both booleans 'visualize' and 'writeToPNG' are false.\n"
                     << "Make sure either one or both are set to true when this function is called to get results.\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }

    // Generate the improvement-curves data file (per-objective improvement [%] + knee metadata)
    this->writeImprovementCurvesFile(fileNameImprovement);

    std::string scriptPath = "/home/tessa/BSO-toolbox-V2/bso/utilities/plot/plot_improvement_curves.py";

    std::stringstream cmd;
    if(!visualize)
    {
        cmd << "MPLBACKEND=Agg ";
    }

    cmd << "python3 "
        << scriptPath << " "
        << fileNameImprovement << " \"" << mDataName << "\" "
        << (visualize ? "1" : "0") << " "
        << (writeToPNG ? "1" : "0") << " \"" << pngName << "\" "
        << (showKnee ? "1" : "0");

    int systemRet = system(cmd.str().c_str());
    if(systemRet == -1)
    {
        std::stringstream errorMessage;
        errorMessage << "\nError, the plotting of the improvement-curves graph did not go as expected and an error has been returned.\n"
                     << "The following fileName was used for plotting: " << fileNameImprovement << "\n"
                     << "(bso/utilities/data_export.cpp). " << std::endl;
        throw std::invalid_argument(errorMessage.str());
    }
} // plotImprovementCurves()

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