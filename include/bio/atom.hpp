#pragma once



#include "glm/vec3.hpp"


#include <string>
#include <vector>
#include <optional>
#include <istream>
#include <filesystem>



struct PDBAtom
{
    size_t id;
    std::string type;
    std::string aa_type;
    size_t aa_id;
    glm::vec3 pos;
    float radius;


    static PDBAtom create(size_t id, std::string type, std::string aa_type, size_t aa_id, glm::vec3 pos, float radius);

    static PDBAtom from_istream(std::istream& atom_istream);

    static std::vector<PDBAtom> atoms_from_istream(std::istream& atoms_istream);
    static std::vector<PDBAtom> atoms_from_filepath(std::filesystem::path atoms_filepath);
};





struct ProteinAtom
{
    std::string symbol;
    std::string aa_type;
    size_t aa_id;
    glm::vec3 pos;
    float radius;


    static ProteinAtom create(std::string symbol, std::string aa_type, size_t aa_id, glm::vec3 pos, float radius);

    static std::optional<ProteinAtom> from_pdb_atom(const PDBAtom& pdb_atom);
};