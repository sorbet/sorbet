#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "parser/prism/PrismArena.h"
#include "parser/prism/prism_xallocator.h"

#include <cstring>
#include <limits>
#include <string>

namespace sorbet::parser::Prism {

TEST_SUITE("PrismArena") {
    TEST_CASE("blocks come from the arena while a scope is active, and from the system otherwise") {
        auto foreignBefore = PrismArena::foreignAllocations();

        void *systemBlock = xmalloc(24);
        REQUIRE_NE(nullptr, systemBlock);
        CHECK_EQ(foreignBefore + 1, PrismArena::foreignAllocations());

        {
            PrismArena arena;
            ArenaScope scope(arena);
            void *arenaBlock = xmalloc(24);
            REQUIRE_NE(nullptr, arenaBlock);
            CHECK_EQ(foreignBefore + 1, PrismArena::foreignAllocations());
            CHECK_EQ(0, reinterpret_cast<uintptr_t>(arenaBlock) % 16);

            // Freeing an arena block is a no-op: the arena frees everything when it goes away.
            std::memset(arenaBlock, 0xAB, 24);
            xfree(arenaBlock);
            CHECK_EQ(0xAB, static_cast<unsigned char *>(arenaBlock)[23]);

            // A system block can be freed from inside a scope too.
            xfree(systemBlock);
        }
        // Once the scope is gone, allocations are system allocations again.
        void *another = xmalloc(8);
        CHECK_EQ(foreignBefore + 2, PrismArena::foreignAllocations());
        xfree(another);
    }

    TEST_CASE("calloc zeroes and realloc preserves contents in either mode") {
        std::string text = "some text that outgrows the first block";
        PrismArena arena;

        // System block, resized while no scope is active.
        auto *system = static_cast<char *>(xcalloc(4, 4));
        REQUIRE_NE(nullptr, system);
        CHECK_EQ(0, system[15]);
        std::memcpy(system, text.data(), 16);
        system = static_cast<char *>(xrealloc(system, text.size() + 1));
        REQUIRE_NE(nullptr, system);
        CHECK_EQ(0, std::memcmp(system, text.data(), 16));

        {
            ArenaScope scope(arena);
            // A system block moved into the arena by a realloc under a scope.
            std::memcpy(system, text.data(), text.size() + 1);
            auto *moved = static_cast<char *>(xrealloc(system, 2 * text.size()));
            REQUIRE_NE(nullptr, moved);
            CHECK_EQ(text, std::string(moved));

            // An arena block resized within the arena.
            auto *arenaBlock = static_cast<char *>(xcalloc(1, 8));
            CHECK_EQ(0, arenaBlock[7]);
            std::memcpy(arenaBlock, "12345678", 8);
            auto *grown = static_cast<char *>(xrealloc(arenaBlock, 64));
            REQUIRE_NE(nullptr, grown);
            CHECK_EQ(0, std::memcmp(grown, "12345678", 8));
            xfree(grown);
            xfree(moved);
        }
    }

    TEST_CASE("impossible requests are refused like malloc refuses them") {
        const size_t huge = std::numeric_limits<size_t>::max() - 8;
        CHECK_EQ(nullptr, xmalloc(huge));
        CHECK_EQ(nullptr, xcalloc(huge, 2));
        PrismArena arena;
        ArenaScope scope(arena);
        CHECK_EQ(nullptr, xmalloc(huge));
        CHECK_EQ(nullptr, xcalloc(huge, 2));
        void *block = xmalloc(8);
        REQUIRE_NE(nullptr, block);
        CHECK_EQ(nullptr, xrealloc(block, huge));
        xfree(block);
    }

    TEST_CASE("the arena grows past its first chunk") {
        PrismArena arena;
        ArenaScope scope(arena);
        std::vector<char *> blocks;
        for (size_t i = 0; i < 4 * PrismArena::FIRST_CHUNK_SIZE / 1024; i++) {
            auto *block = static_cast<char *>(xmalloc(1024));
            REQUIRE_NE(nullptr, block);
            block[0] = static_cast<char>(i);
            blocks.emplace_back(block);
        }
        // Earlier blocks are untouched by later growth.
        for (size_t i = 0; i < blocks.size(); i++) {
            CHECK_EQ(static_cast<char>(i), blocks[i][0]);
        }
    }
}

} // namespace sorbet::parser::Prism
