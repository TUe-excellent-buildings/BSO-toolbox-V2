#ifndef BSO_DATA_PLOT_CPP
#define BSO_DATA_PLOT_CPP

namespace bso { namespace utilities {

data_plot::data_plot()
{

} // ctor()

data_plot::data_plot(std::string fileNameForData, const std::vector<data_point>& data, std::vector<std::string> axisDataTypes, std::string plotName)
:   axisDataTypes(axisDataTypes), dataFileName(fileNameForData), dataName(plotName)
{
	for (const auto& i : data)
	{
		mData.push_back(std::make_shared<data_point>(i));
	}
    this->writeMDataToFile(dataFileName);
} // ctor()

data_plot::~data_plot()
{
	
} // dtor()

void data_plot::writeMDataToFile(std::string fileName)
{
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
    for(const auto& i: mData)
    {
        for(int j=0; j< (*i).size(); j++)
        {     
            output << (*i)(j) << "\t";
        }
        output << std::endl;
    }
}

void data_plot::writeDataToFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName)
{ 
    if(data.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected some data to be stored within the input variable data\n"
										 << "The found size of data is: " << mData.size() << "\n"
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
            output << (*i)(j) << "\t";
        }
        output << std::endl;
    }
} // writeDataToFile()

void data_plot::writeVectorStyleFile(const std::vector<std::shared_ptr<data_point> >& data, std::string fileName)
{
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
                output << (*i)(j) << "\t";
            }
            for(int j=0; j< (*next_i).size(); j++)
            {
                output << ((*next_i)(j)-(*i)(j)) << "\t";
            }
            output << std::endl;
        }
    }
} // writeVectorStyleFile()

void data_plot::plotScatter2D(std::string pngName /*="plot.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/)
{
    if(dataFileName == "" && mData.size() != 0)
    {
        this->writeMDataToFile(dataFileName);
    }
    else if(mData.size() == 0 && dataFileName == "")
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, expected either some data to be stored within the mData variable or a dataFile to plot\n"
										 << "The found size of mData is: " << mData.size() << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Add a check to check weather all data in mData has two data points


    // Send a terminal command to make a plot with GNUplot
    std::string plotDetails = "plot '" + dataFileName  + "' using 1:2 title '" + dataName + "'\"";
    if(visualize)
    {
        std::string TerminalCommand = "gnuplot -persist -e \"set terminal x11; " + plotDetails;

        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << dataFileName << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
    if(writeToPNG)
    {
        std::string TerminalCommand = "gnuplot -e \"set terminal png; set output '" + pngName + "'; " + plotDetails;
        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << dataFileName << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
}

void data_plot::plotParetoFront2D(std::string fileNamePareto /*= "Pareto"*/, std::string fileNameNonPareto /*= "NonPareto"*/, std::string pngName /*="plot.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/)
{    
    if(!visualize && !writeToPNG)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, no plot will be generated since both boolians 'visualize' and 'writeToPNG' are false.\n"
										 << "Make sure either one or both are set to true when this function is called to get results.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(mData.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, no data is proved to plot will be generated since both boolians 'visualize' and 'writeToPNG' are false.\n"
										 << "Make sure either one or both are set to true when this function is called to get results.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    
    // Separate data in pareto front and non-pareto front
    std::vector<std::shared_ptr<data_point> > dataPareto;
    std::vector<std::shared_ptr<data_point> > dataNonPareto;
    this->getParetoFrontData(dataPareto, dataNonPareto);
    
    // Write the pareto and non-pareto data to files
    this->writeDataToFile(dataPareto, fileNamePareto);
    this->writeDataToFile(dataNonPareto, fileNameNonPareto);

    // Make the plot part of the terminal command to make a plot with GNUplot
    std::string plotDetails = "plot '" + fileNameNonPareto  + "' using 1:2 title '" + dataName + "', '" + fileNamePareto + "' using 1:2 title '" + dataName + " on pareto'\"";

    // Perform the plot terminal command to either make a png file of the plot (i.e. bool writeToPNG) or make a visualization (i.e. bool visualize)
    if(visualize)
    {
        std::string TerminalCommand = "gnuplot -persist -e \"set terminal x11; " + plotDetails;

        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << dataFileName << "\n"
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
        
        std::string TerminalCommand = "gnuplot -e \"set terminal png; set output '" + pngName + "'; " + plotDetails;
        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << dataFileName << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
}

void data_plot::plotVector2D(std::string fileNameVector /*= "vectorStyleData"*/, std::string pngName /*="plotVector.png"*/, bool visualize /*= true*/, bool writeToPNG /*= true*/)
{
    if(!visualize && !writeToPNG)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, no plot will be generated since both boolians 'visualize' and 'writeToPNG' are false.\n"
										 << "Make sure either one or both are set to true when this function is called to get results.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    if(mData.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, no data is proved to plot will be generated since both boolians 'visualize' and 'writeToPNG' are false.\n"
										 << "Make sure either one or both are set to true when this function is called to get results.\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }

    // Make a data file with four columns where the first datapoint (x,y) needs to connect to the next two datapoint on the line (xdelta,ydelta) link:http://gnuplot.info/docs_6.0/loc6150.html
    this->writeVectorStyleFile(mData, fileNameVector);

    // Send a terminal command to make a plot with GNUplot
    std::string plotDetails = "plot '" + fileNameVector  + "' using 1:2:3:4 with vectors arrowstyle 3 title '" + dataName + "'\"";
    std::cout << plotDetails << std::endl;

    if(visualize)
    {
        std::string TerminalCommand = "gnuplot -persist -e \"set terminal x11; " + plotDetails;

        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << dataFileName << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
    if(writeToPNG)
    {
        std::string TerminalCommand = "gnuplot -e \"set terminal png; set output '" + pngName + "'; " + plotDetails;
        int systemRet = system(TerminalCommand.c_str());
        if(systemRet == -1)
        {
            std::stringstream errorMessage;
				errorMessage << "\nError, the plotting of the graph did not go as expected and an error has been returned.\n"
										 << "The following fileName was used for plotting: " << dataFileName << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
        }
    }
} // plotArrow2D()

void data_plot::getParetoFrontData(std::vector<std::shared_ptr<data_point> >& dataPareto, std::vector<std::shared_ptr<data_point> >& dataNonPareto)
{
    if(mData.size() == 0)
    {
        std::stringstream errorMessage;
				errorMessage << "\nError, there is no data input given for the separation of data into Pareto and non pareto data.\n"
										 << "The size of mData was found: " << mData.size() << "\n"
                                         << "(bso/utilities/data_plot.cpp). " << std::endl;
				throw std::invalid_argument(errorMessage.str());
    }
    
    for(const auto& i: mData)
    {
        bool isDominated = false;
        for(const auto& j: mData)
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