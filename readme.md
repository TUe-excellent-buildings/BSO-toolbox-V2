# README BSO toolbox

## What is this repository for?
   
This repository contains a C++ library named the Building Spatial design Optimization toolbox (BSO toolbox).
In the first place, the toolbox has been developed to support building spatial design simulation and optimization by providing tools to:
(i) represent and modify building spatial designs,
(ii) generate discipline specific designs from a building spatial design,
(iii) analyze structural designs,
(iv) analyze the building physics of a building (i.e. thermal performance),
(v) analyze data obtained from the evaluations, e.g. clustering and sorting algorithms.
The toolbox itself does not contain any optimization methods, however, this repository will be extended with applications, and these will be related to optimization as well.
The toolbox relates to research in two joint projects by the Department of the Built Environment of Eindhoven University of Technology, and the Leiden Institute of Advanced Computer Science (LIACS), both in The Netherlands.
Key publications within these research projects are given below. These publications provide a lot of background information on the purpose and setup of the code in this repository:
* Ezendam, T.; Hofmeyer, H.; van der Wal, S.; Perevendieva K.G.; and Emmerich, M. T. M.: "Handling Complex Building Geometries for Disciplinary Processing in Conceptual Building Design Simulation and Optimization". Computing in Civil Engineering, 2026, DOI: 10.1061/JCCEE5.CPENG-6922
* Boonstra, S.; Van der Blom, K.; Hofmeyer, H.; Emmerich, M.T.M.: Hybridization of an evolutionary algorithm and simulations of co-evolutionary design processes for early-stage building spatial design optimization, Automation in Construction, Volume 124, Article 103522, pages 1-18, 2021
* Claessens, D.P.H.; Boonstra, S.; Hofmeyer, H.: Spatial Zoning for Better Structural Topology Design and Performance, Advanced Engineering Informatics, Volume 46, 101162, pages 1-16, 2020
* Boonstra, S.; Van der Blom, K.; Hofmeyer, H.; Emmerich, M.T.M.: Conceptual structural system layouts via design response grammars and evolutionary algorithms, Automation in Construction, Volume 116, Article 103009, pages 1-20, 2020
* Van der Blom K.; Boonstra S.; Wang H.; Hofmeyer H.; Emmerich M.T.M. Evaluating Memetic Building Spatial Design Optimisation Using Hypervolume Indicator Gradient Ascent. In: Trujillo L.; Schütze O.; Maldonado Y.; Valle P. (Eds): Numerical and Evolutionary Optimization – NEO 2017, Studies in Computational Intelligence 785, page 62-82, Springer Nature, Cham, Switzerland, 2019
* Boonstra, S.; Van der Blom, K.; Hofmeyer, H.; Emmerich, M.T.M.; Van Schijndel, A.W.M.; De Wilde, P.: Toolbox for super-structured and super-structure free multi-disciplinary building spatial design optimisation, Advanced Engineering Informatics, Volume 36, pages 86-100, 2018
* Hofmeyer, H.; Davila Delgado, J.M.: Coevolutionary and Genetic Algorithm Based Building Spatial and Structural Design, AIEDAM - Artificial Intelligence for Engineering Design, Analysis and Manufacturing, Volume 29, pages 351-370, 2015
* Davila Delgado, J.M.; Hofmeyer, H.: Automated Generation of Structural Solutions based on Spatial Designs, Automation in Construction, Volume 35 (November), pages 528-541, 2013

Moreover, three PhD theses related to the research are:
* Boonstra, S 2020, 'Multi-disciplinary optimization of building spatial designs: co-evolutionary design process simulations, evolutionary algorithms, hybrid approaches', PhD-thesis, Department of the Built Environment, Eindhoven University of Technology, The Netherlands.
* van der Blom, K 2019, 'Multi-objective mixed-integer evolutionary algorithms for building spatial design', PhD-thesis, Leiden Institute of Advanced Computer Science, The Netherlands.
* Davila Delgado, JM 2014, 'Building structural design generation and optimisation including spatial modification', PhD-thesis, Department of the Built Environment, Eindhoven University of Technology, The Netherlands.

Although the first aim of the toolbox was related to research carried out at Eindhoven University of Technology, the contributors now invite all interested researchers to download and use the toolbox; to apply it for innovative design support and optimisation, possibly also in other domains; and to explore and extend its possiblities. This is all possible thanks to the GNU Affero General Public License v3.0. If publications result from using this repository, the contributers would appreciate it if you cite this repository (via the Zenodo DOI) and esspecially the related journal papers above, and not the PhD-theses, although the latter of course provide an excellent source of background information in practice.

## How do I get it set up? ###

### Dependencies
The toolbox is written in C++ and makes use of elements defined in the C++14 standard. The toolbox has been compiled and tested for Linux (Ubuntu 22.04 LTS), and although not included in this release, it is with some minor alterations explained later also compatible with Windows 10 (22H2,19045.2486).

The toolbox depends on the following external source and static/dynamic libraries:
* For linear algebra, the [Eigen](http://eigen.tuxfamily.org) C++ library is used (Linux: last tested is v3.4.0).
* Various utilities (e.g. [Odeint](https://www.odeint.com)) from the [Boost](https://www.boost.org/) C++ library (Linux: last tested is v1.80.0). Precompiling is not needed.
* For constrained delaunay algorithm, the [FADE 2D](https://www.geom.at/) library (linux: last tested is v1.93) was used in this release. Further development did show that the open source library CGAL (version 6.1) could be used as well for this purpose. Alternatively, the FADE 2D v.2.13 worked as well.
* Visualization is written in the OpenGL standard and makes use of freeglut (Linux: last tested is v2.8.1-6).   

Hereafter, an elaborate Linux installation tutorial is given.

### Linux installation
For Linux, a tutuorial for the installation of the dependencies and the toolbox is given in the following steps.
1. Install C++ compiler (GCC) and make
2. Install the Eigen library.
3. Install (and compile) the Boost library.
4. Install FADE 2D
5. Install visualization dependencies (GLUT)
6. Install GIT
7. Clone the repository to your local machine
8. Compile and run the example code.

##### 1. Install C++ compiler (GCC) and make
In this step, a compiler to compile the toolbox into executable code is installed.
Additionally, make is installed, which can parse and execute compiler commands that are given in a so-called makefile

To install the GCC C++ compiler, type in the terminal:
```bash
$ sudo apt install g++
```
To install make, type in the terminal:
```bash
$ sudo apt install make
```

##### 2. Install the Eigen library.
In this step, the [Eigen](http://eigen.tuxfamily.org) library is installed.

Go to the website of the Eigen library (http://eigen.tuxfamily.org) and download the tar.gz of the desired version (e.g. latest stable release).

Extract the package (the downloaded tar.gz) to /usr/include, by typing in the terminal:
```bash
$ sudo tar -xf ~/Downloads/eigen-eigen-[version_number].tar.gz -C /usr/include
```

**OPTIONAL.** Rename the directory so that it is easier to link to the directory, by typing in the terminal:
```bash
$ sudo mv eigen-[version_number] eigen
```

**NOTE.** You may change the directory in which you place Eigen, however, this may have consequences for later steps.

##### 3. Install (and compile) the Boost library.
In this step, the [Boost]() library is installed and parts of it are pre-compiled.

Go to the website of the Boost library (https://boostorg.jfrog.io/artifactory/main/release/1.77.0/source/) and download the tar.gz of the desired version (e.g. latest stable release).

Extract the package (the downloaded tar.gz) to /usr/include, by typing in the terminal:
```bash
$ sudo tar -xf ~/Downloads/boost_[version_number].tar.gz -C /usr/include
```

**OPTIONAL.** Rename the directory so that it is easier to link to the directory, by typing in the terminal:
```bash
$ sudo mv boost_[version_number] boost
```

**NOTE.** You may change the directory in which you place Boost, however, this may have consequences for later steps.

Pre-compile parts of the boost library (e.g. the boost test framework), by typing in the terminal:
```bash
$ cd /usr/include/boost # note that this directory has been defined in the preceding steps
$ sudo ./bootstrap.sh
$ sudo ./b2
$ sudo ./b2 install
```

**NOTE.** Enter the above commands one by one. During Windows 10 installations, it appeared that the compiled parts of Boost are not needed in the toolbox so far, and so also for the Linux installation, precompilation may possibly be treated as an optional step.

##### 4. Install FADE 2D
The toolbox uses the FADE 2D library for constrained Delaunay triangulation. FADE 2D is a separate, licensed third-party dependency; it is not included in this repository. Obtain the Linux FADE 2D v1.93 package from [Geom Software](https://www.geom.at/) and check its license before use. The v1.93 package describes its student license as being for personal, non-commercial scientific research. Commercial use requires a commercial license. Publications using FADE should cite it, and project pages should link to it as required by its license.

Install the GMP development package, a dependency required by FADE:
```bash
sudo apt install libgmp-dev
```

Extract the FADE 2D v1.93 package to a directory that you can keep in place, for example `~/Libraries/fadeRelease_v1.93` for later reference specifications in a 'make file'. Change to that directory in the therminal by using: 
```bash
cd ~/Libraries/fadeRelease_v1.93
```

The v1.93 package does not include a library build specifically for Ubuntu 22.04. Its readme advises choosing a similar Linux distribution when yours is not listed and says the shared libraries support a wide range of distributions. For 64-bit Ubuntu 22.04, use the nearest listed build, `lib_ubuntu21.04_x86_64`, and verify that it works on the target machine. Also, check if the required headers and shared library are present in ~/Libraries/fadeRelease_v1.93 with these following commands (which only work while being in the ~/Libraries/fadeRelease_v1.93 directory): 
```bash
test -f "$PWD/include_fade2d/Fade_2D.h" && echo "Header found" || echo "Header missing"
test -f "$PWD/lib_ubuntu21.04_x86_64/libfade2d.so" && echo "Library found" || echo "Library missing"
```

FADE provides a shared library (`libfade2d.so`). Install that library in the standard local library directory and refresh the dynamic linker cache:
```bash
sudo install -D -m 755 "$FADE_DIR/lib_ubuntu21.04_x86_64/libfade2d.so" /usr/local/lib/libfade2d.so
sudo ldconfig
```

Verify that Ubuntu can find the library:
```bash
ldconfig -p | grep libfade2d
```
The output should include `/usr/local/lib/libfade2d.so`, then the instalation is sucsesfull.

##### 5. Install visualization dependencies (GLUT)
In this step the dependencies for the visualization in the toolbox are installed.
The visualization makes use of OpenGL which is enabled by the GL Utility Toolkit (GLUT)

Install the dependencies by typing in the terminal:
```bash
$ sudo apt install freeglut3-dev
```

##### 6. Install GIT
In this step, the version control software GIT is installed, together with a GUI for GIT.

To install GIT and an GUI type the following in the terminal:
```bash
$ sudo apt install git git-gui
```

##### 7. Clone the repository to your local machine
In this step, the BSO toolbox is cloned to your local machine. This allows you to compile/develop/use the code in the toolbox on your local machine.

In your terminal go to the directory where you would like the toolbox to be placed. For example by typing in the terminal:

```bash
$ cd /usr/include
```

On the homepage of the BSO toolbox click on *clone*.

A dropdown box will appear with two links, copy the link under **Clone with HTTPS** and enter it in the terminal as follows:

```bash
$ git clone <link>
```

The command line on the terminal should then look similar to the line below.

```bash
$ git clone https://github.com/TUe-excellent-buildings/BSO-toolbox.git
```

If the clone was successful, the BSO toolbox is now cloned onto your local machine, e.g. in the directory: `/usr/include/BSO-toolbox`.

##### 8. Compile and run the example.

Examples of how the toolbox can be used has been added to the repository as well (`../BSO-toolbox/examples`). Two examples are provided, each utilizing a diferent representation, either orthogonal or non-orthogonal.

A make file has been added to the directory of the example, which can be used to compile the code.
In the makefile, check if the dependencies (Eigen, Boost, and if applied FADE 2D) are linked to the locations of your local machine.
For instance, check if the line `BOOST = /usr/include/boost` is correctly linking to the Boost library on your local machine.  Also, set FADE_ROOT, in any supplied make file, to the extracted package directory containing both the `include_fade2d` and the `lib_ubuntu...` directories. For example: `FADE=$(HOME)/Libraries/fadeRelease_v1.93`.

The example can then be compiled by typing the following command in the terminal:

```bash
$ make cls clean all
```

Run the resulting 'example' executable by typing the following command in the terminal:

```bash
$ ./example
```

The output should then look as follows.

```bash
> Structural compliance: 4054.5
> Total heating energy: 106.819
> Total cooling energy: 15.9503
```

Note that the directory also includes settings files to define building spatial designs, structural design settings, and building physics settings.
For more information about these the reader is referred to the [paper](https://doi.org/10.1016/j.aei.2018.01.003) in which the toolbox is presented.

### How to run unit tests (general and Linux version, see further below for Windows 10)
To ensure the functionality of the BSO toolbox before and after developing new tools, unit testing is employed.
The unit tests are also included in the repository, and in case of developing/contributing to the BSO toolbox it is adviced to add and test the unit tests regularly.
Depending on the location of the repository on your local machine, moving to the unit test directory may be achieved as follows.

```bash
$ cd /usr/include/BSO-toolbox/unit_tests
```

The unit test directory contains a makefile with which the unit tests can be compiled.
In the makefile, check if the dependencies (Eigen, Boost, and FADE 2D) are linked to the locations of your local machine.
For instance, check if the line `BOOST = /usr/include/boost` is correctly linking to the Boost library on your local machine. And for FADE, in any supplied make file, to the extracted package directory containing both the `include_fade2d` and the `lib_ubuntu...` directories. For example: `FADE=$(HOME)/Libraries/fadeRelease_v1.93`.

To run all tests of the BSO toolbox, make sure the directory mentioned above is the working directory of your terminal and type in the terminal:
```bash
$ make cls clean all
./all_test
```

Possible output of running the unit tests successfully may look as follows:
```bash
------------------------------------------------------------------
* This software uses the Fade2D library under a student license. 
  Commercial use requires a valid commercial license, please visit 
  http://www.geom.at/licensing 

  Geom Software, Bernhard Kornberger
  C++ Freelancer, bkorn@geom.at
  http://www.geom.at

  (Fade2D serial no: 5303992d f08786e8)

Running 470 test cases...

*** No errors detected
```

Alternatively, also sub-parts of the toolbox can be tested. This can be achieved by replacing the `all` argument in the `make` command above by any of the following arguments:
* `all` tests all unit tests
* `ms_space` tests only the unit tests concerning the class ms_space (space in Movable Sizable representation)
* `ms_building` tests only the unit tests concering the class ms_building (building in Movable Sizable representation)
* `ms_N_building` tests only the unit tests concering the class ms_N_building (building in Movable Sizable representation)
* `sc_building` tests only the unit tests concerning the class sc_building (building in SuperCube representation)
* `conformal` tests only the unit tests concerning the cf_building class (conformal building model)
* `trim_cast` tests only the unit tests concerning the trim and cast functions
* `geometry` tests only the unit tests concerning the geometry in utilities
* `delaunay` tests only the unit tests concerning the delaunay triangulations in utilities
* `building_physics` tests only the unit tests concerning the building physics simulation tools
* `structural_design` tests only the unit tests concerning the structural design analysis tools
* `grammar` tests only the unit tests concerning the design grammars
* `visualization` tests only the unit tests concerning the visualization in the toolbox
* `xml` tests only the unit tests concerning XML data im- and export
* `data` tests only the unit tests concerning data points, e.g. Eulerian distance and clustering algorithm

For example to test the geometry package of the toolbox type the following in the terminal:
```bash
$ make clean cls geometry
$ ./geometry_test
```

## Contribution guidelines

The authors of this toolbox encourage others to contribute to the toolbox.
People who would like to contribute are asked to follow the guidelines described in this section.

### Write and run unit tests

To ensure the functionality of the contribution, but also of the existing code, unit tests should be written and run.
Make sure that all existing unit tests are succesfull before sending in your contribution.
Additionally, make sure to write meaningful unit tests to test your contribution.

### Review of code

Contributing to the toolbox is achieved by means of pull requests (also termed merge request on GitLab).
How to open a pull request is explained [here](https://docs.gitlab.com/ee/user/project/merge_requests/getting_started.html).
Once a pull request is opened, an admin of the toolbox can review your code and possibly discuss or modify your code.
If the proposed contribution is considered to be a meaningful addition to the toolbox, an admin can then merge your contribution with the repository.

### Other guidelines

Would you like to propose a change, or are you not sure that your potential contribution will be considered to be meaningfull by the admin?
In this case, you can create an issue, or contact an admin or the project manage (see information below).

## Code accreditation

Parts of the code that are used in the BSO toolbox are based on codes that are originally developed by other authors.
This section is used to clarify which parts of code in the BSO toolbox are based on other codes, and refers to the original work.

#### Method of Moving Asymptotes
The MMA.hpp and MMA.cpp files in the directiory `../BSO-toolbox/bso/structural_design/topology_optimization` are the translation of the MMA-code written in MATLAB by Krister Svanberg.
The original work can be downloaded from http://www.smoptit.se/ under de GNU General Public License.
The user should refer to the academic work of Krister Svanberg when work will be published in which this code is used.
For a description of the original work see this [paper] (https://onlinelibrary.wiley.com/doi/abs/10.1002/nme.1620240207).

## Who do I talk to? ###

###### For technical issues, questions, and interests in the projects:  
Dr. H.(Hèrm) Hofmeyer MSc (project leader Eindhoven University of Technology)  
h.hofmeyer[at]tue.nl  

T.(Tessa) Ezendam MSc (PhD student Eindhoven)  
t.ezendam[at]tue.nl  

###### Other members of the projects:  

Dr. S.(Sjonnie) Boonstra MSc (former PhD student Eindhoven)  
s.boonstra[at]abt.eu

Dr. M.T.M. (Michael) Emmerich MSc (project leader Leiden Institute of Advanced Computer Science (LIACS))  
m.t.m.emmerich[at]liacs.leidenuniv.nl  

K.G. Pereverdieva MSc (PhD student Leiden)  
k.g.pereverdieva[at]liacs.leidenuniv.nl  

Dr. K. van der Blom MSc (former PhD student Leiden)  
koen.vdblom[at]lip6.fr

