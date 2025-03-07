
# Create source files in the repository root
touch "$REPO_ROOT/main.cpp"
touch "$REPO_ROOT/Shader.h" "$REPO_ROOT/Shader.cpp"
touch "$REPO_ROOT/Simulation.h" "$REPO_ROOT/Simulation.cpp"
touch "$REPO_ROOT/CelestialBody.h" "$REPO_ROOT/CelestialBody.cpp"
touch "$REPO_ROOT/SpacetimeGrid.h" "$REPO_ROOT/SpacetimeGrid.cpp"

# Create shader files
touch "$REPO_ROOT/vertex_shader.glsl"
touch "$REPO_ROOT/fragment_shader.glsl"

# Create textures directory and placeholder files
mkdir -p "$REPO_ROOT/textures"
touch "$REPO_ROOT/textures/earth.jpg"
touch "$REPO_ROOT/textures/sun.jpg"

# Display success message
echo "🚀 Gravity Simulator project structure created successfully!"

# Show directory structure
tree "$REPO_ROOT" 2>/dev/null || ls -R "$REPO_ROOT"

# Add files to Git
cd "$REPO_ROOT" || exit
git add .
git commit -m "Setup initial Gravity Simulator structure"
git push origin main  # Change 'mai
