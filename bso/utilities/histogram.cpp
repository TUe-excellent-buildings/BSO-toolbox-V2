#ifndef HISTOGRAM_CPP
#define HISTOGRAM_CPP

namespace bso { namespace utilities {

histogram::histogram() : data()
{
	
} // ctor

histogram::histogram(std::vector<double>& mdata, const std::string& pngName) 
{
    //general histogram

    data = mdata;

    //generate data
    int bins = static_cast<int>(std::round(std::sqrt(data.size())));
    double min_val = *std::min_element(data.begin(), data.end());
    double max_val = *std::max_element(data.begin(), data.end());
    double bin_width = (max_val - min_val) / bins;

    std::vector<int> histogram(bins, 0);

    for (double value : data) 
    {
        int bin_index = std::min(static_cast<int>((value - min_val) / bin_width), bins - 1);
        histogram[bin_index]++;
    }

    // Create a temporary data file for gnuplot
    std::ofstream dataFile("histogram_data.txt");
    for (int i = 0; i < bins; ++i) 
    {
        double lower_bound = min_val + i * bin_width;
        double upper_bound = lower_bound + bin_width;
        dataFile << lower_bound << " " << histogram[i] << std::endl;
    }
    dataFile.close();

    // Gnuplot command to generate histogram
    std::string gnuplotCommand = "gnuplot -e \"set terminal png; set output '" + pngName + "'; "
                            "set boxwidth " + std::to_string(bin_width) + "; "
                            "set style fill solid; "
                            "set xlabel 'Value'; "
                            "set ylabel 'Frequency'; "
                            "plot 'histogram_data.txt' using 1:2 with boxes title 'Histogram';\"";

    // Execute the gnuplot command
    int result = std::system(gnuplotCommand.c_str());
    if (result != 0) 
    {
        std::cerr << "Error: Failed to execute gnuplot command.\n";
    }

    // Optionally, clean up the data file
    std::remove("histogram_data.txt");

} //ctor2

// histogram::histogram(std::vector<std::vector<double>>& performance, const bso::daylight::dl_model& dl, const std::string& iteration) 
// {
//     //histogram for dl data
//
//     /* note 23-10-2024
//     for now adf distribution along BS_EN is not correct, should be based on Dt and Dtm
//     however setupe to make a second column etc is done. 
//     from dl.model the current DF evaluation settings can be found and hence a fixed referenced column can be given to the 0-10 score BS-EN
//     it is ofcourse a bit more difficult than just copy past but look at the dl_model on how this score is calculated normally .
//     */
//    
//     const std::vector<std::vector<double>> RoomInfo = dl.getRoomInfo();
//     //consist out of std::vector<double> SpaceID_MinZ_EI(3);
//
//     double total_min_z = std::numeric_limits<double>::max();
//     double total_max_z = std::numeric_limits<double>::min();
//     for (size_t k = 0; k < RoomInfo.size(); ++k) 
//     {
//         double z_value = RoomInfo[k][1]; 
//         if (z_value < total_min_z) {total_min_z = z_value;}
//         if (z_value > total_max_z) {total_max_z = z_value;}
//     }
//
//     double domain_z;
//     double Bottom_z;
//     double Middle_z;
//     double Top_z;
//     bool no_leveling  = false;
//     if (total_max_z != total_min_z)
//     {
//         domain_z = total_max_z - total_min_z; 
//         Bottom_z = total_min_z + (domain_z / 3); 
//         Middle_z = total_min_z + (domain_z / 3) + (domain_z / 3); 
//         Top_z = total_max_z;
//     }
//     else 
//     {
//         no_leveling = true;
//     }
//
//    
//     std::vector<int> matchingIndexes;
//     for (const auto& room : performance) 
//     {
//         double BS_EN = room[0];                 // ADF value
//         int space_id = static_cast<int>(room[1]);   // Space ID
//         double ADF = room[2];
//
//         for (size_t j =0; j<RoomInfo.size(); ++j)
//         {
//             if (space_id == RoomInfo[j][0])
//             {
//                 matchingIndexes.push_back(j); //retrieve the indexes whixh match this space ID
//             }
//         }
//
//         double space_min_z = std::numeric_limits<double>::max();
//         double space_EI = 1; // by default internal
//         for (const int index : matchingIndexes) 
//         {
//             double z_value = RoomInfo[index][1]; // Get z_value directly
//             if (z_value < space_min_z) {space_min_z = z_value;}
//             if (RoomInfo[index][2] == 0) {space_EI = 0;} // if there is one external complete space is categorized as external. E=0 and I=1
//         }
//
//         matchingIndexes.clear(); //clear it after use for next run to ensure propper grouping based on min_Z
//
//         double space_level;
//         if (!no_leveling)
//         {
//             if (space_min_z <= Bottom_z)                            {space_level = 0;}
//             if (space_min_z <= Middle_z && space_min_z > Bottom_z)  {space_level = 1;}
//             if (space_min_z <= Top_z && space_min_z > Middle_z)     {space_level = 2;}
//         }
//         else
//         {
//             space_level = 0; 
//          }
//
//         std::vector<double> temp(5);
//         temp[0] = space_id;
//         temp[1] = space_level;
//         temp[2] = space_EI;
//         temp[3] = BS_EN;
//         temp[4] = ADF;
//
//         FinalRoomInfo.push_back(temp);
//
//     } //per space
//
//     std::vector<double> group1_data; // Bottom & External
//     std::vector<double> group2_data; // Middle & External
//     std::vector<double> group3_data; // Top & External
//     std::vector<double> group4_data; // Bottom & Internal
//     std::vector<double> group5_data; // Middle & Internal
//     std::vector<double> group6_data; // Top & Internal
//
//     std::vector<double> ADFRoomInfo;
//     for (size_t f = 0; f<FinalRoomInfo.size(); ++f)
//     {
//         if (FinalRoomInfo[f][1] == 0 && FinalRoomInfo[f][2] == 0) {group1_data.push_back(FinalRoomInfo[f][3]);} //make on BS_EN [f][3]
//         if (FinalRoomInfo[f][1] == 1 && FinalRoomInfo[f][2] == 0) {group2_data.push_back(FinalRoomInfo[f][3]);}
//         if (FinalRoomInfo[f][1] == 2 && FinalRoomInfo[f][2] == 0) {group3_data.push_back(FinalRoomInfo[f][3]);}
//         if (FinalRoomInfo[f][1] == 0 && FinalRoomInfo[f][2] == 1) {group4_data.push_back(FinalRoomInfo[f][3]);}
//         if (FinalRoomInfo[f][1] == 1 && FinalRoomInfo[f][2] == 1) {group5_data.push_back(FinalRoomInfo[f][3]);}
//         if (FinalRoomInfo[f][1] == 2 && FinalRoomInfo[f][2] == 1) {group6_data.push_back(FinalRoomInfo[f][3]);}
//         ADFRoomInfo.push_back(FinalRoomInfo[f][4]);
//     }
//
//     // Combine all group data for bin calculation
//     std::vector<double> all_data;
//     all_data.insert(all_data.end(), group1_data.begin(), group1_data.end());
//     all_data.insert(all_data.end(), group2_data.begin(), group2_data.end());
//     all_data.insert(all_data.end(), group3_data.begin(), group3_data.end());
//     all_data.insert(all_data.end(), group4_data.begin(), group4_data.end());
//     all_data.insert(all_data.end(), group5_data.begin(), group5_data.end());
//     all_data.insert(all_data.end(), group6_data.begin(), group6_data.end());
//
//     // Calculate number of bins, min, max, and bin width
//     int bins = 10; //because scoring system is from 0 to 10
//     double min_val = *std::min_element(all_data.begin(), all_data.end());
//     double max_val = *std::max_element(all_data.begin(), all_data.end());
//     double bin_width = 1;//(max_val - min_val) / bins;
//
//     double min_val2 = *std::min_element(ADFRoomInfo.begin(), ADFRoomInfo.end());
//     double max_val2 = *std::max_element(ADFRoomInfo.begin(), ADFRoomInfo.end());
//     double bin_width2 = (max_val2 - min_val2) / bins;
//     if (bin_width == 0) {bin_width = 1; bin_width2 = 1;} // Force bin_width to 1 to avoid division by zero (if all rooms score f.e. 0)
//
//     // Initialize histograms for each group
//     std::vector<int> group_histograms[6]; // Array of 6 histograms
//     for (int i = 0; i < 6; ++i) {group_histograms[i].resize(bins, 0);}
//
//     // Fill histograms for each group
//     for (double value : group1_data) 
//     {
//         int bin_index = std::min(static_cast<int>((value - min_val) / bin_width), bins - 1);
//         group_histograms[0][bin_index]++;
//     }
//
//     for (double value : group2_data) 
//     {
//         int bin_index = std::min(static_cast<int>((value - min_val) / bin_width), bins - 1);
//         group_histograms[1][bin_index]++;
//     }
//
//     for (double value : group3_data) 
//     {
//         int bin_index = std::min(static_cast<int>((value - min_val) / bin_width), bins - 1);
//         group_histograms[2][bin_index]++;
//     }
//
//     for (double value : group4_data) 
//     {
//         int bin_index = std::min(static_cast<int>((value - min_val) / bin_width), bins - 1);
//         group_histograms[3][bin_index]++;
//     }
//
//     for (double value : group5_data) 
//     {
//         int bin_index = std::min(static_cast<int>((value - min_val) / bin_width), bins - 1);
//         group_histograms[4][bin_index]++;
//     }
//
//     for (double value : group6_data) 
//     {
//         int bin_index = std::min(static_cast<int>((value - min_val) / bin_width), bins - 1);
//         group_histograms[5][bin_index]++;
//     }
//
//     // Create a temporary data file for gnuplot
//     std::ofstream dataFile("histogram_data_iteration_" + iteration + ".txt");
//
//     // Write column titles: Bin Lower Bound, ADF Bin Lower Bound, BE, ME, TE, BI, MI, TI
//     dataFile << "BS_EN;ADF;BE;ME;TE;BI;MI;TI" << std::endl;
//
//     // Generate reversed ADF bin lower bounds
//     std::vector<double> adf_lower_bounds(bins);
//     for (int i = 0; i < bins; ++i) {
//         adf_lower_bounds[i] = min_val2 + i * bin_width2;
//     }
//     std::reverse(adf_lower_bounds.begin(), adf_lower_bounds.end()); // Reverse the ADF bounds
//
//     // Write histogram data with reversed ADF bin lower bounds
//     for (int i = 0; i < bins; ++i) 
//     {
//         double lower_bound = min_val + i * bin_width;
//
//         // Format both lower bound and ADF bin lower bound with ',' as decimal separator
//         std::stringstream lower_bound_ss;
//         std::stringstream adf_lower_bound_ss;
//         lower_bound_ss << std::fixed << std::setprecision(2) << lower_bound;
//         adf_lower_bound_ss << std::fixed << std::setprecision(2) << adf_lower_bounds[i];
//
//         std::string lower_bound_str = lower_bound_ss.str();
//         std::string adf_lower_bound_str = adf_lower_bound_ss.str();
//
//         // Replace '.' with ',' for Excel-friendly formatting
//         std::replace(lower_bound_str.begin(), lower_bound_str.end(), '.', ',');
//         std::replace(adf_lower_bound_str.begin(), adf_lower_bound_str.end(), '.', ',');
//
//         // Write lower_bound, ADF lower_bound, and counts for all groups
//         dataFile << lower_bound_str << ";" << adf_lower_bound_str;
//         for (int j = 0; j < 6; ++j) 
//         {
//             dataFile << ";" << group_histograms[j][i];
//         }
//         dataFile << std::endl;
//     }
//
//     dataFile.close();
//    
// } //ctor3

void histogram::exportToCSV(const std::vector<std::vector<double>>& finalRoomInfo, const std::string& filename) 
{
    // Create an output file stream object
    std::ofstream file(filename);

    // Check if the file is open
    if (!file.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
        return;
    }

    // Write the header row for better readability in Excel
    file << "Space_ID;Space_Level;Space_EI;BS_EN;ADF\n";

    // Loop through the finalRoomInfo and write each row to the CSV
    for (const auto& row : finalRoomInfo) {
        for (size_t i = 0; i < row.size(); ++i) {
            // Format the floating-point numbers with a comma as decimal separator
            file << std::fixed << std::setprecision(2) << row[i];

            if (i < row.size() - 1) {
                file << ";";  // Use semicolon as a separator between values
            }
        }
        file << "\n";  // Move to the next line for each row
    }

    // Close the file
    file.close();

    std::cout << "Data exported to " << filename << " successfully." << std::endl;
} //ecport to csv

histogram::histogram(std::vector<std::vector<double>>& performance, const bso::daylight::dl_model& dl, const std::string& iteration) 
{
    //histogram for dl data

    /* note 23-10-2024
    for now adf distribution along BS_EN is not correct, should be based on Dt and Dtm
    however setupe to make a second column etc is done. 
    from dl.model the current DF evaluation settings can be found and hence a fixed referenced column can be given to the 0-10 score BS-EN
    it is ofcourse a bit more difficult than just copy past but look at the dl_model on how this score is calculated normally .
    */
    
    const std::vector<std::vector<double>> RoomInfo = dl.getRoomInfo();
    //consist out of std::vector<double> SpaceID_MinZ_EI(3);

    double total_min_z = std::numeric_limits<double>::max();
    double total_max_z = std::numeric_limits<double>::min();
    for (size_t k = 0; k < RoomInfo.size(); ++k) 
    {
        double z_value = RoomInfo[k][1]; 
        if (z_value < total_min_z) {total_min_z = z_value;}
        if (z_value > total_max_z) {total_max_z = z_value;}
    }

    double domain_z;
    double Bottom_z;
    double Middle_z;
    double Top_z;
    bool no_leveling  = false;
    if (total_max_z != total_min_z)
    {
        domain_z = total_max_z - total_min_z; 
        Bottom_z = total_min_z + (domain_z / 3); 
        Middle_z = total_min_z + (domain_z / 3) + (domain_z / 3); 
        Top_z = total_max_z;
    }
    else 
    {
        no_leveling = true;
    }

    
    std::vector<int> matchingIndexes;
    for (const auto& room : performance) 
    {
        double BS_EN = room[0];                 // ADF value
        int space_id = static_cast<int>(room[1]);   // Space ID
        double ADF = room[2];

        for (size_t j =0; j<RoomInfo.size(); ++j)
        {
            if (space_id == RoomInfo[j][0])
            {
                matchingIndexes.push_back(j); //retrieve the indexes whixh match this space ID
            }
        }

        double space_min_z = std::numeric_limits<double>::max();
        double space_EI = 1; // by default internal
        for (const int index : matchingIndexes) 
        {
            double z_value = RoomInfo[index][1]; // Get z_value directly
            if (z_value < space_min_z) {space_min_z = z_value;}
            if (RoomInfo[index][2] == 0) {space_EI = 0;} // if there is one external complete space is categorized as external. E=0 and I=1
        }

        matchingIndexes.clear(); //clear it after use for next run to ensure propper grouping based on min_Z

        double space_level;
        if (!no_leveling)
        {
            if (space_min_z <= Bottom_z)                            {space_level = 0;}
            if (space_min_z <= Middle_z && space_min_z > Bottom_z)  {space_level = 1;}
            if (space_min_z <= Top_z && space_min_z > Middle_z)     {space_level = 2;}
        }
        else
        {
            space_level = 0; 
         }

        std::vector<double> temp(5);
        temp[0] = space_id;
        temp[1] = space_level;
        temp[2] = space_EI;
        temp[3] = BS_EN;
        temp[4] = ADF;

        FinalRoomInfo.push_back(temp);

    } //per space

    //exportToCSV(FinalRoomInfo, "final_room_info.csv");

    std::vector<double> group1_data; // Bottom & External
    std::vector<double> group2_data; // Middle & External
    std::vector<double> group3_data; // Top & External

    std::vector<double> ADFRoomInfo;
    for (size_t f = 0; f<FinalRoomInfo.size(); ++f)
    {
        // temp[0] = space_id;
        // temp[1] = space_level;
        // temp[2] = space_EI;
        // temp[3] = BS_EN;
        // temp[4] = ADF;

        if (FinalRoomInfo[f][3] == 0)                               {group1_data.push_back(FinalRoomInfo[f][4]);}
        if (FinalRoomInfo[f][3] > 0 && FinalRoomInfo[f][3] <= 5)    {group2_data.push_back(FinalRoomInfo[f][4]);}
        if (FinalRoomInfo[f][3] > 5 && FinalRoomInfo[f][3] <= 10)   {group3_data.push_back(FinalRoomInfo[f][4]);}
    }

    // Combine all group data for bin calculation
    std::vector<double> all_data;
    all_data.insert(all_data.end(), group1_data.begin(), group1_data.end());
    all_data.insert(all_data.end(), group2_data.begin(), group2_data.end());
    all_data.insert(all_data.end(), group3_data.begin(), group3_data.end());

    // Calculate number of bins, min, max, and bin width
    int bins = static_cast<int>(sqrt(all_data.size())); //because scoring system is from 0 to 10
    double min_val = *std::min_element(all_data.begin(), all_data.end());
    double max_val = *std::max_element(all_data.begin(), all_data.end());
    double bin_width = (max_val - min_val) / bins;
    if ((max_val - min_val)<=1){bin_width=bins/5;}

    // Initialize histograms for each group
    std::vector<int> group_histograms[3]; // Array of 6 histograms
    for (int i = 0; i < 3; ++i) {group_histograms[i].resize(bins, 0);}

    // Fill histograms for each group
    for (double value : group1_data) 
    {
        int bin_index = std::min(static_cast<int>((value - min_val) / bin_width), bins - 1);
        group_histograms[0][bin_index]++;
    }

    for (double value : group2_data) 
    {
        int bin_index = std::min(static_cast<int>((value - min_val) / bin_width), bins - 1);
        group_histograms[1][bin_index]++;
    }

    for (double value : group3_data) 
    {
        int bin_index = std::min(static_cast<int>((value - min_val) / bin_width), bins - 1);
        group_histograms[2][bin_index]++;
    }

    // Create a temporary data file for gnuplot
    std::ofstream dataFile("histogram_data_iteration_" + iteration + ".txt");

    // Write column titles: Bin Lower Bound, ADF Bin Lower Bound, BE, ME, TE, BI, MI, TI
    dataFile << "ADF;'0;'1-5;'6-10" << std::endl;


    // Write histogram data with reversed ADF bin lower bounds
    for (int i = 0; i < bins; ++i) 
    {
        double lower_bound = min_val + i * bin_width;

        // Format both lower bound and ADF bin lower bound with ',' as decimal separator
        std::stringstream lower_bound_ss;
        lower_bound_ss << std::fixed << std::setprecision(2) << lower_bound;

        std::string lower_bound_str = lower_bound_ss.str();

        //Replace '.' with ',' for Excel-friendly formatting
        std::replace(lower_bound_str.begin(), lower_bound_str.end(), '.', ',');

        // Write lower_bound, ADF lower_bound, and counts for all groups
        dataFile << lower_bound_str;
        for (int j = 0; j < 3; ++j) 
        {
            dataFile << ";" << group_histograms[j][i];
        }
        dataFile << std::endl;
    }

    dataFile.close();
    
} //ctor4

histogram::~histogram()
{
	
} // dtor

double histogram::computeMean() 
{
    double sum = 0;
    for(double value : data) {
        sum += value;
    }
    return sum / data.size();
} //computerMean

double histogram::computeVariance(double mean) 
{
    double variance = 0;
    for(double value : data) {
        variance += std::pow(value - mean, 2);
    }
    return variance / data.size();
} //computeVariance


} // namespace utilities
} // namespace bso

#endif // HISTOGRAM_CPP
