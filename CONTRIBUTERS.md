## Contributors to the toolbox

###### ir. T. (Tessa) Ezendam
Main contributor and developer of the repository.
Extended all relevant components to support non-orthogonal building spatial designs, including their design and simulation.
Specifically:
* Utilities geometry package: added support for triangles and triangular prisms.
* Spatial design package: developed ms_N_building and extended the conformal package, including cf_building_model, cf_geometry_model, cf_building_entity, cf_triangle, and cf_triprism, to handle ms_N_building models.
* Grammar package: modified the grammars to correctly handle the new cf_building_model and cf_geometry_model, and updated the default BP and SD grammars accordingly.
* Building physics package: adjusted the bp_model to account for triangles and triangular prisms in the cf_geometry_model.
* Structural design package: developed a new meshing method for triangles and adjusted the sd_model to account for triangles and triangular prisms in the cf_geometry_model.
* Visualization: added visualization support for ms_N_building models and extended the cf_building, sd_model, bp_model to visualize the new triangle and triangular prism added to these models. Lastly, the between steps of generating the cf_building model can not be visualized as well.

###### ir. S. (Sjonnie) Boonstra
Main contributor/developer of the repository. Developed the foundation of the BSO toolbox as presented in:
Boonstra, S., & Hofmeyer, H. (2022). BSO Toolbox (Version 1.1.1) [Computer software]. https://doi.org/10.5281/zenodo.3823893
Specifically:
* utilities package: geometry, data_point, clustering, non-dominated sort, trim and cast
* spatial design package: cf_building, ms_building, sc_building, conformal
* building physics package: RC-network model, states, properties, state space system, and bp_model
* structural design package: elements (except formulation of beam, truss, and flat shell elements), components, fea, sd_model, topology optimization (SIMP)
* grammar package: grammar class, default bp and sd grammars, rule sets
* visualization: models of ms_building, cf_building, sc_building, sd_model, and bp_model.

###### dr.ir. H. (Hèrm) Hofmeyer
* Element formulations of beam, truss, and flat shells
* Topology optimization (robust)

###### dr.ir. K. (Koen) van der Blom
* formulation of the supercube representation

###### dr.ir. J.M. (Juan Manuel) Davila Delgado
* formulation of the movable sizable representation

###### ir. T.W. (Tomas) Snel
* Development and implementation of a framework to apply evaluation, analysis/selection and modification techniques in a combinatorial fashion. (not yet included in this repository)

###### ir. Th.Y. (Thijs) de Goede
* Development and implementation of low stiffness flat shell elements to transfer loads on surfaces without underlying structure
* Development and implementation of surface assignments for spaces in the Movable Sizable (MS) representation

###### ir. D.P.H. (Dennis) Claessens
* Development and implementation of zoning of building spatial designs using the building conformal model. (not yet included in this repository)
* Implementation of structural stabilization. (not yet included in this repository)

##### ir. D. (Dennis) Peeten
* Development and implementation of visualization package