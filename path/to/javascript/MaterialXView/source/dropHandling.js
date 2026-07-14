/**
 * Handle dropdown menu events.
 */
export class DropHandling {
  /**
   * Initialize the dropdown menu.
   * @param {HTMLSelectElement} select The dropdown menu select element.
   */
  static init(select) {
    select.addEventListener('change', (event) => {
      const value = event.target.value;
      // Handle the selected value (e.g., update the shader generation system)
      console.log(`Selected value: ${value}`);
    });
  }
}