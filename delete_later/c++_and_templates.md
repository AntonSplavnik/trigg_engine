 Memory Layout Strategy

  DTCM (128KB) - Ultra-fast, CPU-only:
  ├── Stack, local variables
  ├── Hot component data (active viewport entities)
  └── Frame-critical calculations

  AXI SRAM (512KB) - Fast, DMA accessible:
  ├── Render queue (sorted sprites for current frame)
  ├── Active entity indices (what's on screen)
  ├── Component lookup tables (indices into PSRAM)
  └── Temporary working buffers

  PSRAM (16MB) - Slower, but massive:
  ├── Framebuffers (2× ~450KB) = 900KB
  ├── Entity component pools (bulk storage)
  ├── Sprite atlas data
  ├── Level geometry/tiles
  └── Particle systems, effects

  PSRAM Access Characteristics

  Critical: PSRAM is ~4-6x slower than internal SRAM
  - QSPI PSRAM: ~80-100MHz (vs 480MHz CPU)
  - Random access: ~50-100ns latency
  - Sequential reads: Good (burst mode)
  - Solution: Cache-friendly data layouts + batch processing

  Recommended ECS Architecture

  Data-Oriented Design with C++17:

  // ===== Component Storage (PSRAM) =====
  // Structure of Arrays (SoA) - cache-friendly for iteration
  namespace ComponentStorage {
      constexpr size_t MAX_ENTITIES = 2048;  // 16MB = plenty of space

      // Each component type gets its own array
      struct Transform {
          World_space position;
          int16_t z_order;
          uint16_t sprite_id;
      };

      struct Physics {
          Fixed_q16 velocity_x, velocity_y;
          Fixed_q16 acceleration_x, acceleration_y;
          uint8_t collision_layer;
      };

      struct Renderable {
          uint16_t sprite_sheet_id;
          uint8_t animation_frame;
          uint8_t flags;  // flip_x, flip_y, visible, etc.
      };

      // Stored in PSRAM (mark with linker section)
      __attribute__((section(".psram")))
      Transform transforms[MAX_ENTITIES];

      __attribute__((section(".psram")))
      Physics physics[MAX_ENTITIES];

      __attribute__((section(".psram")))
      Renderable renderables[MAX_ENTITIES];

      // Sparse set for entity->component mapping
      uint16_t entity_to_index[MAX_ENTITIES];  // Which entities have which components
      uint32_t component_masks[MAX_ENTITIES];  // Bitfield: which components exist
  }

  // ===== Active Entity Cache (AXI SRAM) =====
  // Only entities in viewport - hot data for current frame
  namespace EntityCache {
      constexpr size_t MAX_ACTIVE = 64;  // Only what's on screen

      struct CachedEntity {
          Transform transform;      // Copy from PSRAM
          Renderable renderable;
          uint16_t entity_id;
      };

      __attribute__((section(".sram_axi")))
      CachedEntity active_entities[MAX_ACTIVE];

      __attribute__((section(".sram_axi")))
      uint16_t active_count = 0;
  }

  // ===== Systems (functions operating on components) =====
  namespace Systems {

      // Physics update: batch read from PSRAM, compute, batch write back
      void update_physics(float dt) {
          // Process in batches to exploit PSRAM burst mode
          constexpr size_t BATCH_SIZE = 32;

          for (size_t batch = 0; batch < MAX_ENTITIES; batch += BATCH_SIZE) {
              // Sequential reads - PSRAM-friendly
              for (size_t i = batch; i < batch + BATCH_SIZE && i < MAX_ENTITIES; ++i) {
                  if (!(component_masks[i] & PHYSICS_MASK)) continue;

                  auto& t = transforms[i];
                  auto& p = physics[i];

                  // All computation in DTCM (registers)
                  t.position.x += p.velocity_x * dt;
                  t.position.y += p.velocity_y * dt;
                  p.velocity_x += p.acceleration_x * dt;
                  // ...
              }
              // Sequential writes - PSRAM burst mode
          }
      }

      // Culling: Build active entity list for rendering
      void cull_and_cache(const Camera& camera) {
          active_count = 0;

          // PSRAM read - sequential through all transforms
          for (uint16_t i = 0; i < MAX_ENTITIES && active_count < MAX_ACTIVE; ++i) {
              if (!(component_masks[i] & RENDERABLE_MASK)) continue;

              const auto& t = transforms[i];
              Screen_space screen = world_to_screen(t.position, camera);

              // Viewport culling
              if (screen.x >= -64 && screen.x < 384 &&
                  screen.y >= -64 && screen.y < 544) {

                  // Copy to fast SRAM cache
                  active_entities[active_count].transform = t;
                  active_entities[active_count].renderable = renderables[i];
                  active_entities[active_count].entity_id = i;
                  active_count++;
              }
          }
      }

      // Render: Only touch cached entities in fast SRAM
      void render() {
          // All data in AXI SRAM - no PSRAM access
          // Sort by z_order (quick, data in fast RAM)
          // ... sorting code ...

          for (uint16_t i = 0; i < active_count; ++i) {
              const auto& e = active_entities[i];
              Screen_space screen = world_to_screen(e.transform.position, camera);

              // Render sprite
              Framebuffer::draw_sprite_alpha(
                  screen.y, sprite_height,
                  screen.x, sprite_width,
                  get_sprite_data(e.renderable.sprite_sheet_id)
              );
          }
      }
  }

  Why C++17 Templates Help Here

  // Template-based component queries - zero runtime cost
  template<typename... Components>
  struct Query {
      // Compile-time mask generation
      static constexpr uint32_t mask = (component_bit<Components>() | ...);

      // Iterator that skips entities without required components
      struct Iterator {
          uint16_t index;

          void operator++() {
              do { ++index; }
              while (index < MAX_ENTITIES &&
                     (component_masks[index] & mask) != mask);
          }

          // Zero-cost access to components
          std::tuple<Components&...> operator*() {
              return {get_component<Components>(index)...};
          }
      };
  };

  // Usage - clean and type-safe
  void update() {
      for (auto [transform, physics] : Query<Transform, Physics>()) {
          transform.position.x += physics.velocity_x;
          // Compiler inlines everything - same as manual loop
      }
  }

  Memory Budget with 16MB PSRAM

  Framebuffers:        900 KB  (2× 450KB)
  2048 entities:      ~400 KB  (200 bytes per entity avg)
  Sprite atlas:       ~2-4 MB  (compressed sprite sheets)
  Particle pools:     ~200 KB  (1000 particles)
  Level data:         ~1-2 MB  (tiles, collision maps)
  Audio buffers:      ~500 KB  (streaming audio)
  Reserve:            ~10 MB   (future expansion)
  -------------------------
  Total:              ~15 MB  (plenty of headroom)

  Performance Expectations

  With this architecture:
  - Update 200 entities: ~2-3ms (PSRAM sequential read/write)
  - Cull 200 → 20 visible: ~1ms (PSRAM read, SRAM write)
  - Render 20 sprites: ~5-8ms (all from SRAM cache)
  - Display transfer: ~8ms (DMA, parallel to next frame)
  - Total frame: ~12-15ms (66-83 FPS)

  Recommendations

  1. Use C++17 - if constexpr, fold expressions, structured bindings eliminate boilerplate
  2. Templates for systems - compile-time polymorphism, zero overhead
  3. Avoid - Still no virtuals, RTTI, exceptions, dynamic allocation
  4. Linker sections - Place component arrays in PSRAM explicitly
  5. Batch PSRAM access - Never random read/write, always sequential
  6. Cache hot data - Copy viewport entities to SRAM each frame

  Hybrid Approach for Your Team

  If teammate wants full modern C++, this is reasonable:
  - Templates + C++17 features = cleaner code, same performance
  - But keep it simple - don't pull in EnTT (overkill)
  - Roll your own ~500 LOC ECS with PSRAM awareness

  Your namespace/function style can coexist:
  - Framebuffer:: namespace - keep as-is
  - Systems:: namespace for ECS systems
  - Gradual migration as needed
