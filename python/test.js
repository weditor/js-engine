//@ts-check

/**
 * 这是一个测试的js文件，会把document字段中的块组合起来。
 * @param {{
 *  document: Array<{block_id: string, text: string}>, 
 *  questions: Array<{question: string, question_type: string}>
 * }} item 
 * @returns {{
 *  document: Array<{block_id: string, text: string}>, 
 *  questions: Array<{question: string, question_type: string}>
 * }}
*/
(item) => { 
    let all_text = item.document.map(item => item.text).join(';');
    item.document = [{ block_id: '0', text: all_text+"。虽然结束了，但是我想加一下废话。" }];
    return item;
}
