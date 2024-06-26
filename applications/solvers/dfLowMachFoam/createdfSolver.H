const labelUList& owner = mesh.owner();
const labelUList& neighbour = mesh.neighbour();
int num_cells = mesh.nCells();
int num_surfaces = neighbour.size();

std::vector<int> boundaryCellIndex;
std::vector<double> boundary_face_vector_init;
std::vector<double> boundary_face_init;
std::vector<double> boundary_deltaCoeffs_init;
std::vector<std::vector<int>> patchTypes;
std::vector<int> patchTypeU, patchTypeY;
int num_boundary_faces = 0;
int patchSize;
forAll(mesh.boundary(), patchi)
{
    labelUList sub_boundary = mesh.boundary()[patchi].faceCells();
    patchSize = sub_boundary.size();
    const vectorField& pSf = mesh.Sf().boundaryField()[patchi];
    const scalarField& pMagSf = mesh.magSf().boundaryField()[patchi];
    const scalarField& pDeltaCoeffs = mesh.nonOrthDeltaCoeffs().boundaryField()[patchi];

    boundaryCellIndex.insert(boundaryCellIndex.end(), &sub_boundary[0], &sub_boundary[0]+patchSize);
    boundary_face_vector_init.insert(boundary_face_vector_init.end(), &pSf[0][0], &pSf[0][0]+3*patchSize);
    boundary_face_init.insert(boundary_face_init.end(), &pMagSf[0], &pMagSf[0]+patchSize);
    boundary_deltaCoeffs_init.insert(boundary_deltaCoeffs_init.end(), &pDeltaCoeffs[0], &pDeltaCoeffs[0]+patchSize);
    num_boundary_faces += patchSize;

    constructBoundarySelector(patchTypeU, U.boundaryField()[patchi].type(), patchSize);
    constructBoundarySelector(patchTypeY, Y[0].boundaryField()[patchi].type(), patchSize);
}
patchTypes.emplace_back(patchTypeU);
patchTypes.emplace_back(patchTypeY);

int num_boundary_cells;

string settingPath;
settingPath = CanteraTorchProperties.subDict("AmgxSettings").lookupOrDefault("UEqnSettingPath", string(""));

#ifdef GPUSolver_
    dfMatrixDataBase dfDataBase(num_surfaces, num_cells, num_boundary_faces, Y.size(), num_boundary_cells, &neighbour[0], &owner[0], &mesh.V()[0], &mesh.surfaceInterpolation::weights()[0], 
    &mesh.Sf()[0][0], &mesh.magSf()[0], &mesh.nonOrthDeltaCoeffs()[0], boundary_face_vector_init, boundary_face_init, boundary_deltaCoeffs_init, boundaryCellIndex, patchTypes);
    dfRhoEqn rhoEqn_GPU(dfDataBase);
    dfUEqn UEqn_GPU(dfDataBase, "dDDI", settingPath);
    dfYEqn YEqn_GPU(dfDataBase, "dDDI", settingPath, inertIndex);
    dfEEqn EEqn_GPU(dfDataBase, "dDDI", settingPath);

    double *ueqn_internalCoeffs_init, *ueqn_boundaryCoeffs_init, *boundary_pressure_init, *boundary_velocity_init,
        *boundary_nuEff_init, *boundary_rho_init, *ueqn_laplac_internalCoeffs_init, *ueqn_laplac_boundaryCoeffs_init, *boundary_phi_init;
    cudaMallocHost(&ueqn_internalCoeffs_init, 3*num_boundary_faces*sizeof(double));
    cudaMallocHost(&ueqn_boundaryCoeffs_init, 3*num_boundary_faces*sizeof(double));
    cudaMallocHost(&ueqn_laplac_internalCoeffs_init, 3*num_boundary_faces*sizeof(double));
    cudaMallocHost(&ueqn_laplac_boundaryCoeffs_init, 3*num_boundary_faces*sizeof(double));
    cudaMallocHost(&boundary_velocity_init, 3*num_boundary_faces*sizeof(double));
    cudaMallocHost(&boundary_pressure_init, num_boundary_faces*sizeof(double));
    cudaMallocHost(&boundary_nuEff_init, num_boundary_faces*sizeof(double));
    cudaMallocHost(&boundary_rho_init, num_boundary_faces*sizeof(double));
    cudaMallocHost(&boundary_phi_init, num_boundary_faces*sizeof(double));

    double *boundary_alphaEff, *boundary_K, *boundary_gradient;
    cudaMallocHost(&boundary_K, num_boundary_faces*sizeof(double));
    cudaMallocHost(&boundary_alphaEff, num_boundary_faces*sizeof(double));
    cudaMallocHost(&boundary_gradient, num_boundary_faces * sizeof(double));

    bool updateBoundaryFields = true; // make sure that the boundary fields do H2D copy at 1st timestep
#endif
