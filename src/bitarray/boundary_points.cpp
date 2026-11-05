#include "bitarray/boundary_points.hpp"


#include "bitarray/bitarray_3d.hpp"


#include <variant>
#include <optional>
#include <span>
#include <functional>
#include <limits>

#include <iostream> // [todo] remove



constexpr static size_t st1 = 1;
constexpr static size_t stmax = std::numeric_limits<size_t>::max();



struct lazy_bitarray_3d_boundary_points_wrapper
{
    static std::vector<glm::size3_t> lazy_bitarray_3d_boundary_points(
        const lazy_bitarray_3d& lazy_bitarray
    );

    static void compute_bps(
        const lazy_bitarray_3d_node& node,
        std::vector<glm::size3_t>& bps,
        glm::size3_t offset,
        glm::size3_t size,
        std::span<glm::size3_t> layers_sizing,
        const lazy_bitarray_3d_node& n_xn,
        const lazy_bitarray_3d_node& n_xp,
        const lazy_bitarray_3d_node& n_yn,
        const lazy_bitarray_3d_node& n_yp,
        const lazy_bitarray_3d_node& n_zn,
        const lazy_bitarray_3d_node& n_zp
    );

    static const lazy_bitarray_3d_node& get_child(const lazy_bitarray_3d_node& node, glm::size3_t index);

    static bool get_bit(const lazy_bitarray_3d_node& node, glm::size3_t index);

    static bool skip_node(const lazy_bitarray_3d_node& node);

    static lazy_bitarray_3d_node fake_false_node;
    static lazy_bitarray_3d_node fake_true_node;
};



std::vector<glm::size3_t> lazy_bitarray_3d_boundary_points_wrapper::lazy_bitarray_3d_boundary_points(const lazy_bitarray_3d& lazy_bitarray)
{
    std::vector<glm::size3_t> bps;

    compute_bps(
        lazy_bitarray.data,
        bps,
        glm::size3_t(0),
        lazy_bitarray.size,
        lazy_bitarray.layers_sizing,
        fake_false_node, fake_false_node, fake_false_node, fake_false_node, fake_false_node, fake_false_node
    );

    return bps;
}

void lazy_bitarray_3d_boundary_points_wrapper::compute_bps(
    const lazy_bitarray_3d_node& node,
    std::vector<glm::size3_t>& bps,
    glm::size3_t offset,
    glm::size3_t size,
    std::span<glm::size3_t> layers_sizing,
    const lazy_bitarray_3d_node& n_xn,
    const lazy_bitarray_3d_node& n_xp,
    const lazy_bitarray_3d_node& n_yn,
    const lazy_bitarray_3d_node& n_yp,
    const lazy_bitarray_3d_node& n_zn,
    const lazy_bitarray_3d_node& n_zp
) {
    size_t variant_index = node.data.index();

    if (layers_sizing.size() == 0) {
        if (variant_index == 0) {
            if (!std::get<0>(node.data)) {
                return;
            }

            size_t xn = 0;
            for (size_t y = 0; y < size.y; y++) {
                for (size_t z = 0; z < size.z; z++) {
                    bool b_xn = get_bit(n_xn, glm::size3_t(stmax, y, z));
                    bool b_xp = true;

                    bool b_yn = (y == 0)          ? get_bit(n_yn, glm::size3_t(xn, stmax, z))     : true;
                    bool b_yp = (y == size.y - 1) ? get_bit(n_yp, glm::size3_t(xn, 0    , z))     : true;
                    bool b_zn = (z == 0)          ? get_bit(n_zn, glm::size3_t(xn, y    , stmax)) : true;
                    bool b_zp = (z == size.z - 1) ? get_bit(n_zp, glm::size3_t(xn, y    , 0))     : true;

                    if (!b_xn || !b_xp || !b_yn || !b_yp || !b_zn || !b_zp) {
                        bps.push_back(offset + glm::size3_t(xn, y, z));
                    }
                }
            }

            size_t xp = size.x - 1;
            for (size_t y = 0; y < size.y; y++) {
                for (size_t z = 0; z < size.z; z++) {
                    bool b_xn = true;
                    bool b_xp = get_bit(n_xp, glm::size3_t(0, y, z));
                    bool b_yn = (y == 0)          ? get_bit(n_yn, glm::size3_t(xp, stmax, z))     : true;
                    bool b_yp = (y == size.y - 1) ? get_bit(n_yp, glm::size3_t(xp, 0    , z))     : true;
                    bool b_zn = (z == 0)          ? get_bit(n_zn, glm::size3_t(xp, y    , stmax)) : true;
                    bool b_zp = (z == size.z - 1) ? get_bit(n_zp, glm::size3_t(xp, y    , 0))     : true;

                    if (!b_xn || !b_xp || !b_yn || !b_yp || !b_zn || !b_zp) {
                        bps.push_back(offset + glm::size3_t(xp, y, z));
                    }
                }
            }

            size_t yn = 0;
            for (size_t x = 1; x < size.x - 1; x++) {
                for (size_t z = 0; z < size.z; z++) {

                    bool b_xn = true;
                    bool b_xp = true;
                    bool b_yn = get_bit(n_yn, glm::size3_t(x, stmax, z));
                    bool b_yp = true;
                    bool b_zn = (z == 0)          ? get_bit(n_zn, glm::size3_t(x, yn, stmax)) : true;
                    bool b_zp = (z == size.z - 1) ? get_bit(n_zp, glm::size3_t(x, yn, 0))     : true;

                    if (!b_xn || !b_xp || !b_yn || !b_yp || !b_zn || !b_zp) {
                        bps.push_back(offset + glm::size3_t(x, yn, z));
                    }
                }
            }

            size_t yp = 0;
            for (size_t x = 1; x < size.x - 1; x++) {
                for (size_t z = 0; z < size.z; z++) {

                    bool b_xn = true;
                    bool b_xp = true;
                    bool b_yn = true;
                    bool b_yp = get_bit(n_yp, glm::size3_t(x, 0, z));
                    bool b_zn = (z == 0)          ? get_bit(n_zn, glm::size3_t(x, yp, stmax)) : true;
                    bool b_zp = (z == size.z - 1) ? get_bit(n_zp, glm::size3_t(x, yp, 0))     : true;

                    if (!b_xn || !b_xp || !b_yn || !b_yp || !b_zn || !b_zp) {
                        bps.push_back(offset + glm::size3_t(x, yp, z));
                    }
                }
            }

            size_t zn = 0;
            for (size_t x = 1; x < size.x - 1; x++) {
                for (size_t y = 1; y < size.y - 1; y++) {
                    bool b_xn = true;
                    bool b_xp = true;
                    bool b_yn = true;
                    bool b_yp = true;
                    bool b_zn = get_bit(n_zn, glm::size3_t(x, y, stmax));
                    bool b_zp = true;

                    if (!b_xn || !b_xp || !b_yn || !b_yp || !b_zn || !b_zp) {
                        bps.push_back(offset + glm::size3_t(x, y, zn));
                    }
                }
            }

            size_t zp = 0;
            for (size_t x = 1; x < size.x - 1; x++) {
                for (size_t y = 1; y < size.y - 1; y++) {
                    bool b_xn = true;
                    bool b_xp = true;
                    bool b_yn = true;
                    bool b_yp = true;
                    bool b_zn = true;
                    bool b_zp = get_bit(n_zp, glm::size3_t(x, y, zp));

                    if (!b_xn || !b_xp || !b_yn || !b_yp || !b_zn || !b_zp) {
                        bps.push_back(offset + glm::size3_t(x, y, zp));
                    }
                }
            }
        } else {
            const bitarray_3d& bitarray = std::get<2>(node.data);

            for (size_t x = 0; x < size.x; x++) {
                for (size_t y = 0; y < size.y; y++) {
                    for (size_t z = 0; z < size.z; z++) {
                        if (bitarray.get_bit_unsafe(glm::size3_t(x, y, z))) {
                            bool b_xn = (x == 0)          ? get_bit(n_xn, glm::size3_t(stmax, y    , z))     : bitarray.get_bit_unsafe(glm::size3_t(x - 1, y, z));
                            bool b_xp = (x == size.x - 1) ? get_bit(n_xp, glm::size3_t(0    , y    , z))     : bitarray.get_bit_unsafe(glm::size3_t(x + 1, y, z));
                            bool b_yn = (y == 0)          ? get_bit(n_yn, glm::size3_t(x    , stmax, z))     : bitarray.get_bit_unsafe(glm::size3_t(x, y - 1, z));
                            bool b_yp = (y == size.y - 1) ? get_bit(n_yp, glm::size3_t(x    , 0    , z))     : bitarray.get_bit_unsafe(glm::size3_t(x, y + 1, z));
                            bool b_zn = (z == 0)          ? get_bit(n_zn, glm::size3_t(x    , y    , stmax)) : bitarray.get_bit_unsafe(glm::size3_t(x, y, z - 1));
                            bool b_zp = (z == size.z - 1) ? get_bit(n_zp, glm::size3_t(x    , y    , 0))     : bitarray.get_bit_unsafe(glm::size3_t(x, y, z + 1));

                            if (!b_xn || !b_xp || !b_yn || !b_yp || !b_zn || !b_zp) {
                                bps.push_back(offset + glm::size3_t(x, y, z));
                            }
                        }
                    }
                }
            }
        }
        
        return;
    }

    std::span<glm::size3_t> child_layers_sizing(layers_sizing.last(layers_sizing.size() - 1));

    if (variant_index == 0) {
        if (!std::get<0>(node.data)) {
            return;
        }

        bool xn_skip = skip_node(n_xn);
        bool xp_skip = skip_node(n_xp);
        bool yn_skip = skip_node(n_yn);
        bool yp_skip = skip_node(n_yp);
        bool zn_skip = skip_node(n_zn);
        bool zp_skip = skip_node(n_zp);

        glm::size3_t child_size(layers_sizing[0]);
        glm::size3_t child_count((size - st1) / child_size + st1);
        glm::size3_t last_child_size((size - st1) % child_size + st1);

        if (!xn_skip) {
            for (size_t y = 1; y < child_count.y - 1; y++) {
                for (size_t z = 1; z < child_count.z - 1; z++) {
                    compute_bps(
                        fake_true_node,
                        bps,
                        offset + glm::size3_t(0, y, z) * child_size,
                        glm::size3_t(1, child_size.y, child_size.z),
                        child_layers_sizing,
                        get_child(n_xn, glm::size3_t(stmax, y, z)),
                        fake_true_node,
                        fake_true_node,
                        fake_true_node,
                        fake_true_node,
                        fake_true_node
                    );
                }
            }
        }

        if (!xp_skip) {
            for (size_t y = 1; y < child_count.y - 1; y++) {
                for (size_t z = 1; z < child_count.z - 1; z++) {
                    compute_bps(
                        fake_true_node,
                        bps,
                        offset + glm::size3_t(child_count.x - 1, y, z) * child_size,
                        glm::size3_t(1, child_size.y, child_size.z),
                        child_layers_sizing,
                        fake_true_node,
                        get_child(n_xp, glm::size3_t(0, y, z)),
                        fake_true_node,
                        fake_true_node,
                        fake_true_node,
                        fake_true_node
                    );
                }
            }
        }

        if (!yn_skip) {
            for (size_t x = 1; x < child_count.x - 1; x++) {
                for (size_t z = 1; z < child_count.z - 1; z++) {
                    compute_bps(
                        fake_true_node,
                        bps,
                        offset + glm::size3_t(x, 0, z) * child_size,
                        glm::size3_t(child_size.x, 1, child_size.z),
                        child_layers_sizing,
                        fake_true_node,
                        fake_true_node,
                        get_child(n_yn, glm::size3_t(x, stmax, z)),
                        fake_true_node,
                        fake_true_node,
                        fake_true_node
                    );
                }
            }
        }

        if (!yp_skip) {
            for (size_t x = 1; x < child_count.x - 1; x++) {
                for (size_t z = 1; z < child_count.z - 1; z++) {
                    compute_bps(
                        fake_true_node,
                        bps,
                        offset + glm::size3_t(x, child_count.y - 1, z) * child_size,
                        glm::size3_t(child_size.x, 1, child_size.z),
                        child_layers_sizing,
                        fake_true_node,
                        fake_true_node,
                        fake_true_node,
                        get_child(n_yp, glm::size3_t(x, 0, z)),
                        fake_true_node,
                        fake_true_node
                    );
                }
            }
        }

        if (!zn_skip) {
            for (size_t x = 1; x < child_count.x - 1; x++) {
                for (size_t y = 1; y < child_count.y - 1; y++) {
                    compute_bps(
                        fake_true_node,
                        bps,
                        offset + glm::size3_t(x, y, 0) * child_size,
                        glm::size3_t(child_size.x, child_size.y, 1),
                        child_layers_sizing,
                        fake_true_node,
                        fake_true_node,
                        fake_true_node,
                        fake_true_node,
                        get_child(n_zn, glm::size3_t(x, y, stmax)),
                        fake_true_node
                    );
                }
            }
        }

        if (!zp_skip) {
            for (size_t x = 1; x < child_count.x - 1; x++) {
                for (size_t y = 1; y < child_count.y - 1; y++) {
                    compute_bps(
                        fake_true_node,
                        bps,
                        offset + glm::size3_t(x, y, child_count.z - 1) * child_size,
                        glm::size3_t(child_size.x, child_size.y, 1),
                        child_layers_sizing,
                        fake_true_node,
                        fake_true_node,
                        fake_true_node,
                        fake_true_node,
                        fake_true_node,
                        get_child(n_zp, glm::size3_t(x, y, 0))
                    );
                }
            }
        }

        if (!xn_skip || !yn_skip) {
            for (size_t z = 1; z < child_count.z - 1; z++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(0, 0, z) * child_size,
                    child_size,
                    child_layers_sizing,
                    get_child(n_xn, glm::size3_t(stmax, 0, z)),
                    fake_true_node,
                    get_child(n_yn, glm::size3_t(0, stmax, z)),
                    fake_true_node,
                    fake_true_node,
                    fake_true_node
                );
            }
        }

        if (!xn_skip || !yp_skip) {
            for (size_t z = 1; z < child_count.z - 1; z++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(0, child_count.y - 1, z) * child_size,
                    glm::size3_t(child_size.x, last_child_size.y, child_size.z),
                    child_layers_sizing,
                    get_child(n_xn, glm::size3_t(stmax, stmax, z)),
                    fake_true_node,
                    fake_true_node,
                    get_child(n_yp, glm::size3_t(0, 0, z)),
                    fake_true_node,
                    fake_true_node
                );
            }
        }

        if (!xp_skip || !yn_skip) {
            for (size_t z = 1; z < child_count.z - 1; z++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(child_count.x - 1, 0, z) * child_size,
                    glm::size3_t(last_child_size.x, child_size.y, child_size.z),
                    child_layers_sizing,
                    fake_true_node,
                    get_child(n_xp, glm::size3_t(0, stmax, z)),
                    get_child(n_yn, glm::size3_t(stmax, 0, z)),
                    fake_true_node,
                    fake_true_node,
                    fake_true_node
                );
            }
        }

        if (!xp_skip || !yp_skip) {
            for (size_t z = 1; z < child_count.z - 1; z++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(child_count.x - 1, child_count.y - 1, z) * child_size,
                    glm::size3_t(last_child_size.x, last_child_size.y, child_size.z),
                    child_layers_sizing,
                    fake_true_node,
                    get_child(n_xp, glm::size3_t(stmax, stmax, z)),
                    fake_true_node,
                    get_child(n_yp, glm::size3_t(0, 0, z)),
                    fake_true_node,
                    fake_true_node
                );
            }
        }

        if (!xn_skip || !zn_skip) {
            for (size_t y = 1; y < child_count.y - 1; y++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(0, y, 0) * child_size,
                    child_size,
                    child_layers_sizing,
                    get_child(n_xn, glm::size3_t(stmax, y, 0)),
                    fake_true_node,
                    fake_true_node,
                    fake_true_node,
                    get_child(n_zn, glm::size3_t(0, y, stmax)),
                    fake_true_node
                );
            }
        }

        if (!xn_skip || !zp_skip) {
            for (size_t y = 1; y < child_count.y - 1; y++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(0, y, child_count.z - 1) * child_size,
                    glm::size3_t(child_size.x, child_size.y, last_child_size.z),
                    child_layers_sizing,
                    get_child(n_xn, glm::size3_t(stmax, y, stmax)),
                    fake_true_node,
                    fake_true_node,
                    fake_true_node,
                    fake_true_node,
                    get_child(n_zp, glm::size3_t(0, y, 0))
                );
            }
        }

        if (!xp_skip || !zn_skip) {
            for (size_t y = 1; y < child_count.y - 1; y++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(child_count.x - 1, y, 0) * child_size,
                    glm::size3_t(last_child_size.x, child_size.y, child_size.z),
                    child_layers_sizing,
                    fake_true_node,
                    get_child(n_xp, glm::size3_t(0, y, 0)),
                    fake_true_node,
                    fake_true_node,
                    get_child(n_zn, glm::size3_t(stmax, y, stmax)),
                    fake_true_node
                );
            }
        }

        if (!xp_skip || !zp_skip) {
            for (size_t y = 1; y < child_count.y - 1; y++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(child_count.x - 1, y, child_count.z - 1) * child_size,
                    glm::size3_t(last_child_size.x, child_size.y, last_child_size.z),
                    child_layers_sizing,
                    fake_true_node,
                    get_child(n_xp, glm::size3_t(0, y, stmax)),
                    fake_true_node,
                    fake_true_node,
                    fake_true_node,
                    get_child(n_zp, glm::size3_t(stmax, y, 0))
                );
            }
        }

        if (!yn_skip || !zn_skip) {
            for (size_t x = 1; x < child_count.x - 1; x++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(x, 0, 0) * child_size,
                    child_size,
                    child_layers_sizing,
                    fake_true_node,
                    fake_true_node,
                    get_child(n_yn, glm::size3_t(x, stmax, 0)),
                    fake_true_node,
                    get_child(n_zn, glm::size3_t(x, 0, stmax)),
                    fake_true_node
                );
            }
        }

        if (!yn_skip || !zp_skip) {
            for (size_t x = 1; x < child_count.x - 1; x++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(x, 0, child_count.y - 1) * child_size,
                    glm::size3_t(child_size.x, child_size.y, last_child_size.z),
                    child_layers_sizing,
                    fake_true_node,
                    fake_true_node,
                    get_child(n_yn, glm::size3_t(x, stmax, stmax)),
                    fake_true_node,
                    fake_true_node,
                    get_child(n_zp, glm::size3_t(x, 0, 0))
                );
            }
        }

        if (!yp_skip || !zn_skip) {
            for (size_t x = 1; x < child_count.x - 1; x++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(x, child_count.y - 1, 0) * child_size,
                    glm::size3_t(child_size.x, last_child_size.y, child_size.z),
                    child_layers_sizing,
                    fake_true_node,
                    fake_true_node,
                    fake_true_node,
                    get_child(n_yp, glm::size3_t(x, 0, 0)),
                    get_child(n_zn, glm::size3_t(x, stmax, stmax)),
                    fake_true_node
                );
            }
        }

        if (!yp_skip || !zp_skip) {
            for (size_t x = 1; x < child_count.x - 1; x++) {
                compute_bps(
                    fake_true_node,
                    bps,
                    offset + glm::size3_t(x, child_count.y - 1, child_count.y - 1) * child_size,
                    glm::size3_t(child_size.x, last_child_size.y, last_child_size.z),
                    child_layers_sizing,
                    fake_true_node,
                    fake_true_node,
                    fake_true_node,
                    get_child(n_yp, glm::size3_t(x, 0, stmax)),
                    fake_true_node,
                    get_child(n_zp, glm::size3_t(x, stmax, 0))
                );
            }
        }

        if (!xn_skip || !yn_skip || !zn_skip) {
            compute_bps(
                fake_true_node,
                bps,
                offset + glm::size3_t(0, 0, 0) * child_size,
                glm::size3_t(child_size.x, child_size.y, child_size.z),
                child_layers_sizing,
                get_child(n_xn, glm::size3_t(stmax, 0, 0)),
                fake_true_node,
                get_child(n_yn, glm::size3_t(0, stmax, 0)),
                fake_true_node,
                get_child(n_zn, glm::size3_t(0, 0, stmax)),
                fake_true_node
            );
        }

        if (!xn_skip || !yn_skip || !zp_skip) {
            compute_bps(
                fake_true_node,
                bps,
                offset + glm::size3_t(0, 0, child_count.z - 1) * child_size,
                glm::size3_t(child_size.x, child_size.y, last_child_size.z),
                child_layers_sizing,
                get_child(n_xn, glm::size3_t(stmax, 0, stmax)),
                fake_true_node,
                get_child(n_yn, glm::size3_t(0, stmax, stmax)),
                fake_true_node,
                fake_true_node,
                get_child(n_zp, glm::size3_t(0, 0, 0))
            );
        }

        if (!xn_skip || !yp_skip || !zn_skip) {
            compute_bps(
                fake_true_node,
                bps,
                offset + glm::size3_t(0, child_count.y - 1, 0) * child_size,
                glm::size3_t(child_size.x, last_child_size.y, child_size.z),
                child_layers_sizing,
                get_child(n_xn, glm::size3_t(stmax, stmax, 0)),
                fake_true_node,
                fake_true_node,
                get_child(n_yp, glm::size3_t(0, 0, 0)),
                get_child(n_zn, glm::size3_t(0, stmax, stmax)),
                fake_true_node
            );
        }

        if (!xn_skip || !yp_skip || !zp_skip) {
            compute_bps(
                fake_true_node,
                bps,
                offset + glm::size3_t(0, child_count.y - 1, child_count.z - 1) * child_size,
                glm::size3_t(child_size.x, last_child_size.y, last_child_size.z),
                child_layers_sizing,
                get_child(n_xn, glm::size3_t(stmax, stmax, stmax)),
                fake_true_node,
                fake_true_node,
                get_child(n_yp, glm::size3_t(0, 0, stmax)),
                fake_true_node,
                get_child(n_zp, glm::size3_t(0, stmax, 0))
            );
        }

        if (!xp_skip || !yn_skip || !zn_skip) {
            compute_bps(
                fake_true_node,
                bps,
                offset + glm::size3_t(child_count.x - 1, 0, 0) * child_size,
                glm::size3_t(last_child_size.x, child_size.y, child_size.z),
                child_layers_sizing,
                fake_true_node,
                get_child(n_xp, glm::size3_t(0, 0, 0)),
                get_child(n_yn, glm::size3_t(stmax, stmax, 0)),
                fake_true_node,
                get_child(n_zn, glm::size3_t(stmax, 0, stmax)),
                fake_true_node
            );
        }

        if (!xp_skip || !yn_skip || !zp_skip) {
            compute_bps(
                fake_true_node,
                bps,
                offset + glm::size3_t(child_count.x - 1, 0, child_count.z - 1) * child_size,
                glm::size3_t(last_child_size.x, child_size.y, last_child_size.z),
                child_layers_sizing,
                fake_true_node,
                get_child(n_xp, glm::size3_t(0, 0, stmax)),
                get_child(n_yn, glm::size3_t(stmax, stmax, stmax)),
                fake_true_node,
                fake_true_node,
                get_child(n_zp, glm::size3_t(stmax, 0, 0))
            );
        }

        if (!xp_skip || !yp_skip || !zn_skip) {
            compute_bps(
                fake_true_node,
                bps,
                offset + glm::size3_t(child_count.x - 1, child_count.y - 1, 0) * child_size,
                glm::size3_t(last_child_size.x, last_child_size.y, child_size.z),
                child_layers_sizing,
                fake_true_node,
                get_child(n_xp, glm::size3_t(0, stmax, 0)),
                fake_true_node,
                get_child(n_yp, glm::size3_t(stmax, 0, 0)),
                get_child(n_zn, glm::size3_t(stmax, stmax, stmax)),
                fake_true_node
            );
        }

        if (!xp_skip || !yp_skip || !zp_skip) {
            compute_bps(
                fake_true_node,
                bps,
                offset + glm::size3_t(child_count.x - 1, child_count.y - 1, child_count.z - 1) * child_size,
                glm::size3_t(last_child_size.x, last_child_size.y, last_child_size.z),
                child_layers_sizing,
                fake_true_node,
                get_child(n_xp, glm::size3_t(0, stmax, stmax)),
                fake_true_node,
                get_child(n_yp, glm::size3_t(stmax, 0, stmax)),
                fake_true_node,
                get_child(n_zp, glm::size3_t(stmax, stmax, 0))
            );
        }
    } else {
        const lazy_bitarray_3d_node_array& node_array = std::get<1>(node.data);

        glm::size3_t child_count(node_array.child_count);
        glm::size3_t child_size(node_array.child_size);
        glm::size3_t last_child_size(node_array.last_child_size);

        for (size_t x = 0; x < child_count.x; x++) {
            for (size_t y = 0; y < child_count.y; y++) {
                for (size_t z = 0; z < child_count.z; z++) {
                    const lazy_bitarray_3d_node& child_n_xn = (x == 0)                 ? get_child(n_xn, glm::size3_t(stmax, y    , z))     : node_array.nodes[x - 1 + y * child_count.x + z * child_count.x * child_count.y];
                    const lazy_bitarray_3d_node& child_n_xp = (x == child_count.x - 1) ? get_child(n_xp, glm::size3_t(0    , y    , z))     : node_array.nodes[x + 1 + y * child_count.x + z * child_count.x * child_count.y];
                    const lazy_bitarray_3d_node& child_n_yn = (y == 0)                 ? get_child(n_yn, glm::size3_t(x    , stmax, z))     : node_array.nodes[x + (y - 1) * child_count.x + z * child_count.x * child_count.y];
                    const lazy_bitarray_3d_node& child_n_yp = (y == child_count.y - 1) ? get_child(n_yp, glm::size3_t(x    , 0    , z))     : node_array.nodes[x + (y + 1) * child_count.x + z * child_count.x * child_count.y];
                    const lazy_bitarray_3d_node& child_n_zn = (z == 0)                 ? get_child(n_zn, glm::size3_t(x    , y    , stmax)) : node_array.nodes[x + y * child_count.x + (z - 1) * child_count.x * child_count.y];
                    const lazy_bitarray_3d_node& child_n_zp = (z == child_count.z - 1) ? get_child(n_zp, glm::size3_t(x    , y    , 0))     : node_array.nodes[x + y * child_count.x + (z + 1) * child_count.x * child_count.y];

                    compute_bps(
                        node_array.nodes[x + y * child_count.x + z * child_count.x * child_count.y],
                        bps,
                        offset + glm::size3_t(x, y, z) * child_size,
                        glm::size3_t((x == child_count.x - 1) ? last_child_size.x : child_size.x, (y == child_count.y - 1) ? last_child_size.y : child_size.y, (z == child_count.z - 1) ? last_child_size.z : child_size.z),
                        child_layers_sizing,
                        child_n_xn,
                        child_n_xp,
                        child_n_yn,
                        child_n_yp,
                        child_n_zn,
                        child_n_zp
                    );
                }
            }
        }
    }
}

const lazy_bitarray_3d_node& lazy_bitarray_3d_boundary_points_wrapper::get_child(const lazy_bitarray_3d_node& node, glm::size3_t index)
{
    if (node.data.index() == 0) {
        return node;
    }

    const lazy_bitarray_3d_node_array& node_array = std::get<1>(node.data);

    if (index.x == stmax) { index.x = node_array.child_count.x - 1; }
    if (index.y == stmax) { index.y = node_array.child_count.y - 1; }
    if (index.z == stmax) { index.z = node_array.child_count.z - 1; }

    return node_array.nodes[index.x + index.y * node_array.child_count.x + index.z * node_array.child_count.x * node_array.child_count.y];
}

bool lazy_bitarray_3d_boundary_points_wrapper::get_bit(const lazy_bitarray_3d_node& node, glm::size3_t index)
{
    if (node.data.index() == 0) {
        return std::get<0>(node.data);
    }

    const bitarray_3d& bitarray = std::get<2>(node.data);

    if (index.x == stmax) { index.x = bitarray.size.x - 1; }
    if (index.y == stmax) { index.y = bitarray.size.y - 1; }
    if (index.z == stmax) { index.z = bitarray.size.z - 1; }

    return bitarray.get_bit_unsafe(index);
}

bool lazy_bitarray_3d_boundary_points_wrapper::skip_node(const lazy_bitarray_3d_node& node)
{
    return node.data.index() == 0 && std::get<0>(node.data);
}



lazy_bitarray_3d_node lazy_bitarray_3d_boundary_points_wrapper::fake_false_node(false);
lazy_bitarray_3d_node lazy_bitarray_3d_boundary_points_wrapper::fake_true_node(true);



std::vector<glm::size3_t> lazy_bitarray_3d_boundary_points(const lazy_bitarray_3d& lazy_bitarray)
{
    return lazy_bitarray_3d_boundary_points_wrapper::lazy_bitarray_3d_boundary_points(lazy_bitarray);
}