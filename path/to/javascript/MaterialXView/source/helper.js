/**
 * Helper functions for the dropdown menu.
 */
export class DropdownHelper {
  /**
   * Get the list of enumerated values from the spec/definitions.
   * @returns {Promise<string[]>} A promise resolving to an array of enumerated values.
   */
  static async getEnumeratedValues() {
    try {
      const response = await fetch('/spec/definitions');
      const data = await response.json();
      return data.enumeratedValues;
    } catch (error) {
      console.error('Error fetching enumerated values:', error);
      return [];
    }
  }

  /**
   * Create a dropdown menu option from an enumerated value.
   * @param {string} value The enumerated value.
   * @returns {HTMLOptionElement} A dropdown menu option element.
   */
  static createOption(value) {
    const option = document.createElement('option');
    option.value = value;
    option.textContent = value;
    return option;
  }
}