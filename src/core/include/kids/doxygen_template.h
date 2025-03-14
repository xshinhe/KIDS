/**@file        doxygen_template.h
 * @brief       This is a template file with using doxygen
 * @details     Please follow the minimal doxygen rules. Note that the \@brief and
 *              \@details documentation can be automatically determined by heading
 *              of the markdown structure.
 *              So don't use \@brief and \@details unless in the head of a file.
 *
 * @author      [author] [author2] [author3]
 * @date        [latest-date]
 * @version     [version]
 * @copyright   [copyright]
 **********************************************************************************
 * @par revision [logs]:
 * <table>
 * <tr><th> Date    <th> Version    <th> Author    <th> Description
 * <tr><td>[date]   <td>[version]   <td>[author]   <td> [commit]
 * </table>
 *
 **********************************************************************************
 */

#ifndef DOXYGEN_TEMPLATE_EXAMPLE_H
#define DOXYGEN_TEMPLATE_EXAMPLE_H

#include <algorithm>

/**
 * This is an example namespace.
 */
namespace DOXYGEN_TEMPLATE_EXAMPLE {

const char[] var = "You are the best!";  ///< variable documentation

/**
 * This is an example function
 */
inline int fun();

/**
 * This is an example enum.
 * # principle colors
 * red, green, blue
 */
enum Color {
    red,    ///< red
    green,  ///< green
    blue    ///< blue
};

/**
 * This is an example class
 *
 * @author [author]
 * @date 2000-01-01
 *
 */
class Example {
   public:
    /**
     * @param[in] test this is the only parameter of this test function. It does nothing!
     *
     * # Supported elements
     *
     * These elements have been tested with the custom CSS.
     *
     * ## Tables
     *
     * <div class="tabbed">
     *
     * - <b class="tab-title">Basic</b>
     *   This theme supports normal markdown tables:<br>
     *   | Item | Title | Description           | More                                       |
     *   |-----:|-------|-----------------------|--------------------------------------------|
     *   |    1 | Foo   | A placeholder         | Some lorem ipsum to make this table wider. |
     *   |    2 | Bar   | Also a placeholder    | More lorem ipsum.                          |
     *   |    3 | Baz   | The third placeholder | More lorem ipsum.                          |
     * - <b class="tab-title">Centered</b>
     *   <center>
     *   A table can be centered with the `<center>` html tag:<br>
     *   | Item | Title | Description           | More                                       |
     *   |-----:|-------|-----------------------|--------------------------------------------|
     *   |    1 | Foo   | A placeholder         | Some lorem ipsum to make this table wider. |
     *   |    2 | Bar   | Also a placeholder    | More lorem ipsum.                          |
     *   |    3 | Baz   | The third placeholder | More lorem ipsum.                          |
     *   </center>
     * - <b class="tab-title">Stretched</b>
     *   A table wrapped in `<div class="full_width_table">` fills the full page width.
     *   <div class="full_width_table">
     *   | Item | Title | Description           | More                                       |
     *   |-----:|-------|-----------------------|--------------------------------------------|
     *   |    1 | Foo   | A placeholder         | Some lorem ipsum to make this table wider. |
     *   |    2 | Bar   | Also a placeholder    | More lorem ipsum.                          |
     *   |    3 | Baz   | The third placeholder | More lorem ipsum.                          |
     *   </div>
     *   **Caution**: This will break the overflow scrolling support!
     * - <b class="tab-title">Complex</b>
     *   Complex [Doxygen tables](https://www.doxygen.nl/manual/tables.html) are also supported as seen in @ref
     * multi_row "this example":<br> <table> <caption id="multi_row">Complex table</caption> <tr><th>Column 1 <th>Column
     * 2        <th>Column 3 <tr><td rowspan="2">cell row=1+2,col=1<td>cell row=1,col=2<td>cell row=1,col=3 <tr><td
     * rowspan="2">cell row=2+3,col=2                    <td>cell row=2,col=3 <tr><td>cell row=3,col=1 <td>cell
     * row=3,col=3
     *   </table>
     * - <b class="tab-title">Overflow Scrolling</b> The table content is scrollable if the table gets too wide.<br>
     *   | first_column | second_column | third_column | fourth_column | fifth_column | sixth_column | seventh_column |
     * eighth_column | ninth_column |
     *   |--------------|---------------|--------------|---------------|--------------|--------------|----------------|---------------|--------------|
     *   | 1            | 2             | 3            | 4             | 5            | 6            | 7              |
     * 8             | 9            |
     * - <b class="tab-title">Images</b>A table can contain images:<br>
     *   | Column 1                  | Column 2                                        |
     *   |---------------------------|-------------------------------------------------|
     *   | ![doxygen](testimage.png) | ← the image should not be inverted in dark-mode |
     *
     *
     * </div>
     *
     * ## Diagrams
     *
     * Graphviz diagrams support dark mode and can be scrolled once they get too wide:
     *
     * \dot Graphviz with a caption
     *  digraph example {
     *      node [fontsize="12"];
     *      rankdir="LR"
     *      a -> b -> c -> d -> e -> f -> g -> h -> i -> j -> k;
     *  }
     *  \enddot
     *
     * ## Lists
     *
     * - element 1
     * - element 2
     *
     * 1. element 1
     *    ```
     *    code in lists
     *    ```
     * 2. element 2
     *
     * ## Quotes
     *
     * > Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt
     * > ut labore et dolore magna aliqua. Vitae proin sagittis nisl rhoncus mattis rhoncus urna neque viverra.
     * > Velit sed ullamcorper morbi tincidunt ornare.
     * >
     * > Lorem ipsum dolor sit amet consectetur adipiscing elit duis.
     * *- jothepro*
     *
     * ## Code block
     *
     * ```cpp
     * auto x = "code within md fences";
     * ```
     *
     * @code{.cpp}
     * // code within @code block
     * while(true) {
     *    auto example = std::make_shared<Example>(5);
     *    example->test("test");
     * }
     * @endcode
     *
     *     // code within indented code block
     *     auto test = std::shared_ptr<Example(5);
     *
     *
     * Inline `code` elements in a text. *Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor
     * incididunt ut labore et dolore magna aliqua.* This also works within multiline text and does not break the
     * `layout`.
     *
     * ## Groups
     * @group
     *
     * ## Special hints
     *
     * @warning this is a warning only for demonstration purposes
     *
     * @note this is a note to show that notes work. They can also include `code`:
     * @code{.c}
     * void this_looks_awesome();
     * @endcode
     *
     * @bug example bug
     *
     * @deprecated None of this will be deprecated, because it's beautiful!
     *
     * @invariant This is an invariant
     *
     * @pre This is a precondition
     *
     * @post This is a postcondition
     *
     * @todo This theme is never finished!
     *
     * @remark This is awesome!
     *
     */
    std::string test(const std::string &test);

    Example();
};

/**@defgroup test
 * @addtogroup test This are collection of functions
 * This is a group example
 */
/// @{
int group_fun1(){return 0};
int group_fun2(){return 0};
int group_fun3(){return 0};
/// @}

};  // namespace DOXYGEN_TEMPLATE_EXAMPLE


#endif  // DOXYGEN_TEMPLATE_EXAMPLE_H
