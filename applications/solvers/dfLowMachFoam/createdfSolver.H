const labelUList& owner = mesh.owner();
const labelUList& neighbour = mesh.neighbour();
int num_cells = mesh.nCells();
int num_surfaces = neighbour.size();

std::vector<int> boundaryCellIndex;
std::vector<double> boundary_face_vector_init;
int num_boundary_faces = 0;
int patchSize;
int offset = 0;
forAll(mesh.boundary(), patchi)
{
    labelUList sub_boundary = mesh.boundary()[patchi].faceCells();
    patchSize = sub_boundary.size();
    const vectorField& pSf = mesh.Sf().boundaryField()[patchi];

    boundaryCellIndex.insert(boundaryCellIndex.end(), &sub_boundary[0], &sub_boundary[0]+patchSize);
    boundary_face_vector_init.insert(boundary_face_vector_init.end()+offset, &pSf[0][0], &pSf[0][0]+3*patchSize);
    num_boundary_faces += patchSize;
}
int num_boundary_cells;
dfMatrix UEqn_csr(num_surfaces, num_cells, num_boundary_faces, num_boundary_cells, &neighbour[0], &owner[0], &mesh.V()[0], &mesh.surfaceInterpolation::weights()[0], 
&mesh.Sf()[0][0], boundary_face_vector_init, boundaryCellIndex);
double *ueqn_internalCoeffs_init = new double[3*num_boundary_faces];
double *ueqn_boundaryCoeffs_init = new double[3*num_boundary_faces];
double *boundary_pressure_init = new double[num_boundary_faces];