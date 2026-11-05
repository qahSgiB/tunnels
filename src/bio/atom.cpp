#include "bio/atom.hpp"


#include "bio/element.hpp"


#include <fstream>
#include <optional>
#include <string>



PDBAtom PDBAtom::create(size_t id, std::string type, std::string aa_type, size_t aa_id, glm::vec3 pos, float radius)
{
    return PDBAtom {
        .id = id,
        .type = std::move(type),
        .aa_type = std::move(aa_type),
        .aa_id = aa_id,
        .pos = pos,
        .radius = radius
    };
}

PDBAtom PDBAtom::from_istream(std::istream& atom_istream)
{
    size_t id;
    std::string type;
    std::string aa_type;
    size_t aa_id;
    glm::vec3 pos;
    float radius;

    atom_istream >> id;
    atom_istream >> type;
    atom_istream >> aa_type;
    atom_istream.ignore(2);
    atom_istream >> aa_id;
    atom_istream.ignore(4);
    atom_istream >> pos.x;
    atom_istream >> pos.y;
    atom_istream >> pos.z;
    atom_istream.ignore(6);
    atom_istream >> radius;

    return PDBAtom {
        .id = id,
        .type = std::move(type),
        .aa_type = std::move(aa_type),
        .aa_id = aa_id,
        .pos = pos,
        .radius = radius
    };
}



std::vector<PDBAtom> PDBAtom::atoms_from_istream(std::istream& atoms_istream)
{
    std::vector<PDBAtom> atoms;

    std::string line_type;

    while (atoms_istream.peek() != std::istream::traits_type::eof()) {
        if (atoms_istream.peek() == '\n') {
            atoms_istream.get();
            continue;
        }

        atoms_istream >> line_type;

        if (line_type == "ATOM") {
            atoms.push_back(PDBAtom::from_istream(atoms_istream));
        }

        atoms_istream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return atoms;
}

std::vector<PDBAtom> PDBAtom::atoms_from_filepath(std::filesystem::path atoms_filepath)
{
    std::ifstream atoms_file(atoms_filepath);
    atoms_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    return atoms_from_istream(atoms_file);
}



ProteinAtom ProteinAtom::create(std::string symbol, std::string aa_type, size_t aa_id, glm::vec3 pos, float radius)
{
    return ProteinAtom {
        .symbol = symbol,
        .aa_type = aa_type,
        .aa_id = aa_id,
        .pos = pos,
        .radius = radius,
    };
}

std::optional<ProteinAtom> ProteinAtom::from_pdb_atom(const PDBAtom &pdb_atom)
{
    std::string symbol = std::string(1, pdb_atom.type[0]);
    auto element_iter = Element::world_symbol_to_element.find(symbol);
    if (element_iter == Element::world_symbol_to_element.end()) {
        return std::nullopt;
    }
    const Element& element = element_iter->second.get();

    return ProteinAtom {
        .symbol = symbol,
        .aa_type = pdb_atom.aa_type,
        .aa_id = pdb_atom.aa_id,
        .pos = pdb_atom.pos,
        .radius = element.radius,
    };
}