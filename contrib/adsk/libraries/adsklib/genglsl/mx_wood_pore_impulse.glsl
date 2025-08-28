#include "../../stdlib/genglsl/lib/mx_noise.glsl"

// Wyvill kernel with pre-squared input.
float wyvillsq(float rsq)
{
	float tmp = 1.0 - rsq;
	return rsq >= 1.0 ? 0.0 : tmp * tmp * tmp;
}
// Compute impulse for wood pore effects which can't be done by node graph. This is essentially a re-implementation of the 
// weight2DNeighborImpulses function in the 3ds max advance wood source code.
void mx_wood_pore_impulse(vec3 position, float wood_weight, float seed, float pore_cell_dim, float pore_radius, out float weight)
{
  weight = 0.0;
  if (wood_weight <= 0.0) 
  {
    return;
  }
  float weighted_pore_radius = pore_radius * wood_weight;
  
  // Determine the boundaries of the pore cells that may affect us, given the pore radius and
  // cell dimensions.
  float x_left  = floor((position.x - weighted_pore_radius) / pore_cell_dim);
  float x_right = floor((position.x + weighted_pore_radius) / pore_cell_dim);
  float y_left  = floor((position.y - weighted_pore_radius) / pore_cell_dim);
  float y_right = floor((position.y + weighted_pore_radius) / pore_cell_dim);

  // Sum the wyvill impulse from all potentially contributing cells.
  float inverse_radius_sq = 1.0 / (weighted_pore_radius * weighted_pore_radius);
  for (int j = int(y_left); j <= int(y_right); j++) 
  {
    for (int i = int(x_left); i <= int(x_right); i++) 
    {
      // Determine where the pore is in the cell
      vec2 offset = mx_cell_noise_vec3(vec2(i + seed * 37, j)).xy;
      vec2 pore_position = (offset + vec2(float(i), float(j))) * pore_cell_dim;
  
      // Compute the distance to the pore and use it for the wyvill impulse.
      vec2 diff = pore_position - vec2(position.x,position.y);
      float diff_sq = dot(diff, diff);
      weight += wyvillsq(diff_sq * inverse_radius_sq);
    }
  }       
}

