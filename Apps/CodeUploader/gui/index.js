// current image file object reference
let imageFile = null;

let e_image_md5sum = null;
let e_image_appid = null;
let e_image_path = null;
let e_image_size = null;
let e_button = null;
let e_dropzone = null;
let is_uploading = false;

function waiting( state ) {
    console.log(`waiting(${state})`);
    if( state ) {
        document.body.style.cursor = 'wait';
        e_dropzone.style.cursor = 'wait';
    } else {
        document.body.style.cursor = null;
        e_dropzone.style.cursor = null;
    }
}

function UploadFile() {
    
    // test: not currently uploading a file
    if( is_uploading ) {
        console.error("UploadFile() - called while in use!");
        return;
    }

    // verify: image file exists
    if( imageFile == null ) {
        console.error("UploadFile() - called with non-existant imageFile!");
        return;
    }

    // verify: image file size is reasonable
    if( imageFile.size < 600*1024 ) {
        imageFile == null;
        alert("Error image file must be >= 600KiB in size!");
        return;
    }

    console.log(`Upload: ${imageFile.name}`);

    // is_uploading = true;
    waiting( true );

    const uri = "/upload";
    const xhr = new XMLHttpRequest();
    const fd = new FormData();

    xhr.open("POST", uri, true);
    xhr.onreadystatechange = () => {
        if (xhr.readyState === 4 && xhr.status === 200) {
            alert(xhr.responseText); // handle response.
        }
    };

    // append form data elements
    fd.append( 'appID', imageFile.appid );
    fd.append( 'appMD5', imageFile.md5sum );    
    fd.append( 'appSize', imageFile.size );
    fd.append( 'appImage', imageFile );

    // update display elements
    e_image_appid.innerText = imageFile.appid;
    e_image_path.innerText = './' + imageFile.fullpath;
    e_image_size.innerText = `${(imageFile.size / 1024).toFixed(2)} Kib`;
    e_image_md5sum.innerText = imageFile.md5sum;

    e_dropzone.setAttribute('loaded','true');    
    e_button.setAttribute('enabled','true');

    // Initiate a multipart/form-data upload
    try {
        xhr.send(fd);
    } catch(e) {
        console.log('Error uploading file:',e);
        alert('Error uploading file:',e);
    }

    console.log('Done uploading file...');
    waiting( false );
    is_uploading = false;
}

window.onload = () => {
    console.log("window.onload()");

    e_image_md5sum = document.getElementById('image_md5sum');
    e_image_appid = document.getElementById('image_appid');
    e_image_path = document.getElementById('image_path');
    e_image_size = document.getElementById('image_size');
    e_dropzone = document.getElementById('dropzone');    
    e_button = document.getElementById('button');


    window.addEventListener('drop', (event) => { 
        event.preventDefault(); 
    });

    window.addEventListener('dragover', (event) => { 
        event.dataTransfer.dropEffect = 'none';
        event.preventDefault(); 
    });

    const dropzone = document.getElementById("dropzone");
    dropzone.ondragover = dropzone.ondragenter = (event) => {
        event.stopPropagation();
        event.preventDefault();
    }

    dropzone.ondrop = (event) => {        
        event.stopPropagation();
        event.preventDefault();

        // reset imageFile and control states
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        imageFile = null;
        e_image_md5sum.innerText = "";        
        e_image_appid.innerText = "";
        e_image_path.innerText = "";
        e_image_size.innerText = "";
        e_dropzone.setAttribute('loaded','false');        
        e_button.setAttribute('enabled','false');

        // test that only one folder was dropped
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        const filesArray = event.dataTransfer.files;
        if( filesArray.length > 1 ) {
            alert("Only one file may be dragged and dropped at a time!");
            return;
        }
        
        waiting(true);

        // traverseFileTree(item, path = "") :: recursively walk file tree
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // TODO: limit depth recursion to five folders, add relevant error message
        let is_traversing = false;
        function traverseFileTree(item, path = "", depth = 0 ) {
            path = path || "";
            
            if( depth >= 5 ) {
                console.error(`traverseFileTree(...) ${depth} limit reached`);                
                return;
            }

            // exit: already found image file
            if( imageFile != null ) return;

            // file: parse full path name and check for appid folder
            if (item.isFile) {
                item.file(function(file) {
                    let appid = parseInt( path.split('/').reverse()[1] );
                    if( isNaN( appid ) || appid < 2 ) return;
                    if( file.name === 'esp32c3.app' ) {
                        imageFile = file;
                        imageFile.fullpath = path + file.name;
                        imageFile.appid = appid;
                        console.log("id:", imageFile.appid );
                        console.log("path", imageFile.fullpath );
                    }
                });
            } 

            // dir: iterate sub-folders and recurse
            else if (item.isDirectory) {
                var dirReader = item.createReader();
                dirReader.readEntries(function(entries) {
                    for (var i=0; i<entries.length; i++) {
                        traverseFileTree(entries[i], path + item.name + "/", depth + 1 );
                    }
                });
            }

            if( !depth ) is_traversing = false;
        }

        // get transfer item and walk file tree
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        var items = event.dataTransfer.items;
        var item = items[0].webkitGetAsEntry();
        if (item) {
            is_traversing = true;
            traverseFileTree( item );
        }

        // ugly hack to get around async behavior of directory transversal
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        let timer = window.setInterval( () => {

            if( is_traversing ) return;
            window.clearInterval( timer );

            if( imageFile == null ) {
                alert("Could not find Pocuter image file 'esp32c3.app'!");
                waiting(false);
                return;
            }

            var reader = new FileReader();            
            reader.onload = function(event) {
                var wordArray = CryptoJS.lib.WordArray.create(this.result);
                var md5 = CryptoJS.MD5(wordArray);
                imageFile.md5sum = md5;

                console.log('md5:', imageFile.md5sum );
                console.log('size:', imageFile.size );

                UploadFile();
            };
            reader.readAsArrayBuffer( imageFile );
        }, 200 );
    };
}