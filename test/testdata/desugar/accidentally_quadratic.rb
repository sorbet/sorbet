# typed: true

# This test used to cause quadratic behavior in Desugar.
# Contributed by Harry Doan, https://github.com/manhhung741

module FakeData::DemoCheckpointSampleDoc
  CONTENTS = JSON.parse(%Q(
{
  "type": "doc",
  "attrs": {
    "layout": "us_letter_portrait",
    "padding": null,
    "width": null
  },
  "content": [{
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 30
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#4b4b96"
        }
      }],
      "text": "Checkpoint Samples"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "In order to help you prepare for Project Instruction, we have provided these generic checkpoint samples. Examining these samples will help you practice using the feedback features and examine the cognitive skills rubric in more detail. "
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "These samples have been intentionally simplified for the purpose of summer training: the work is shorter, does not feature grammatical issues, or features fewer cognitive skills than is typical for a Checkpoint."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#e6553c"
        }
      }],
      "text": "Note: These samples were created by our PD Team and are "
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#e6553c"
        }
      }, {
        "type": "underline"
      }],
      "text": "inspired"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#e6553c"
        }
      }],
      "text": " by student work. They do "
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#e6553c"
        }
      }, {
        "type": "underline"
      }],
      "text": "not"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#e6553c"
        }
      }],
      "text": " feature any work produced by students."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "link",
        "attrs": {
          "href": "#id.gwwiwceqo8km",
          "rel": "noopener noreferrer nofollow",
          "target": "",
          "title": null
        }
      }, {
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#1155cc"
        }
      }, {
        "type": "underline"
      }],
      "text": "Science"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 1,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Course "
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "- 7th grade Integrated Science "
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 1,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Project "
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "- Chemical Reactions "
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 1,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 2"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": " - Planning Your Experiment"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "link",
        "attrs": {
          "href": "#id.f4hecsjcd653",
          "rel": "noopener noreferrer nofollow",
          "target": "",
          "title": null
        }
      }, {
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#1155cc"
        }
      }, {
        "type": "underline"
      }],
      "text": "English Language Arts"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 1,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Course "
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "- 7th grade English Language Arts "
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 1,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Project "
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "- From Story to Screen "
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 1,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": " - Compare/Contrast Essay Outline"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "link",
        "attrs": {
          "href": "#id.2eut15jp8ddz",
          "rel": "noopener noreferrer nofollow",
          "target": "",
          "title": null
        }
      }, {
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#1155cc"
        }
      }, {
        "type": "underline"
      }],
      "text": "History / Social Science"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 1,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Course "
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "- 7th grade Medieval History "
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 1,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Project "
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "- "
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Feudal Honor Codes & Values "
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 1,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": " - Purpose of Social Codes"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.bc66d8bc994c787ececbcc0c1eb88d45e6fbb661",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.0",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Zachary             Course - Integrated Science 7         Project - Chemical Reactions"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 109,
        "rotate": null,
        "src": "https://lh4.googleusercontent.com/sd7A45VJC5SnPyeeQMX2Yh6T0yEyWWzu5qFEIpP1_i19_IS_v_7qDl_Wok3RRLJ2hTcr5YFgc7dXJW9b3Q4gDE8v4Xw_ucT_hoc3XA-IF8mW88bglGSFvC_ddedlkXJOiSIjaZPp",
        "title": null,
        "width": 94
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 112,
        "rotate": null,
        "src": "https://lh5.googleusercontent.com/Wo4RJuwW8_PR5BMiUUcGk7eLK4UGQPsvR60rJvSKmgTZ0bPCByaz7wq573ylSe8Jlkzz1szyYlCmW2Vl0M9G58DNUM-miAZ74RlmErrI3q_bf2bmeEMT94L4D0-ujR2GAr5fEac_",
        "title": null,
        "width": 98
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 111,
        "rotate": null,
        "src": "https://lh6.googleusercontent.com/MDRhHpwIdCErJsYgABjkyykobV4YDn7RXe4bswEAH4LvcuRtXCN5kYtn62mcYtffzXBnoCZIa4QLVoY-_aDLMNoOkkCgc0EMpNxzMJn6Vrs-Um78m5tWXPsxp-Tk86kCXbheiaru",
        "title": null,
        "width": 99
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "id.gwwiwceqo8km",
        "visible": true
      }
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 24
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 2: Planning Your Experiment"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "For this checkpoint, you will define your hypothesis."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.1a13f401ced518402e8a692de045cdd956832016",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.1",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What Question will your group try to answer in this experiment?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "(Copy / paste from Checkpoint 1.)"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "When making elephant toothpaste, what will be the difference in the amount of bubbles if we use 3 ounces or 2 ounces of dish soap?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.9b500398276a223b5c635ad4915757ca24f14b61",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.2",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What"
          }, {
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": " is your group's hypothesis?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "We hypothesize that the reaction will produce more bubbles when we use 3 ounces of dish soap because the dehydration of hydrogen peroxide produces oxygen, which makes the dish soap foam up."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.f271394b21825391c1dbf8c96c1d425795ce6783",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.3",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Eliza             Course - Integrated Science 7         Project - Chemical Reactions"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 109,
        "rotate": null,
        "src": "https://lh4.googleusercontent.com/sd7A45VJC5SnPyeeQMX2Yh6T0yEyWWzu5qFEIpP1_i19_IS_v_7qDl_Wok3RRLJ2hTcr5YFgc7dXJW9b3Q4gDE8v4Xw_ucT_hoc3XA-IF8mW88bglGSFvC_ddedlkXJOiSIjaZPp",
        "title": null,
        "width": 94
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 112,
        "rotate": null,
        "src": "https://lh5.googleusercontent.com/Wo4RJuwW8_PR5BMiUUcGk7eLK4UGQPsvR60rJvSKmgTZ0bPCByaz7wq573ylSe8Jlkzz1szyYlCmW2Vl0M9G58DNUM-miAZ74RlmErrI3q_bf2bmeEMT94L4D0-ujR2GAr5fEac_",
        "title": null,
        "width": 98
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 111,
        "rotate": null,
        "src": "https://lh6.googleusercontent.com/MDRhHpwIdCErJsYgABjkyykobV4YDn7RXe4bswEAH4LvcuRtXCN5kYtn62mcYtffzXBnoCZIa4QLVoY-_aDLMNoOkkCgc0EMpNxzMJn6Vrs-Um78m5tWXPsxp-Tk86kCXbheiaru",
        "title": null,
        "width": 99
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 24
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 2: Planning Your Experiment"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "For this checkpoint, you will define your hypothesis."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.87e63152e662ebf5a0b9a4390d027fe382de1f09",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.4",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What Question will your group try to answer in this experiment?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "(Copy / paste from Checkpoint 1.)"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "When making elephant toothpaste, what will be the difference in the amount of bubbles if we use 0.25 packets of yeast or 0.5 ounces of yeast?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.b174e3eecec5cf1626b45bb963b1dd1c5446aa3b",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.5",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What"
          }, {
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": " is your group's hypothesis?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "We predict that the reaction will be bigger when we use more yeast."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.650a3ccf399359189fae7ad4c91aff90aae4bd9e",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.6",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Li             Course - Integrated Science 7         Project - Chemical Reactions"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 109,
        "rotate": null,
        "src": "https://lh4.googleusercontent.com/sd7A45VJC5SnPyeeQMX2Yh6T0yEyWWzu5qFEIpP1_i19_IS_v_7qDl_Wok3RRLJ2hTcr5YFgc7dXJW9b3Q4gDE8v4Xw_ucT_hoc3XA-IF8mW88bglGSFvC_ddedlkXJOiSIjaZPp",
        "title": null,
        "width": 94
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 112,
        "rotate": null,
        "src": "https://lh5.googleusercontent.com/Wo4RJuwW8_PR5BMiUUcGk7eLK4UGQPsvR60rJvSKmgTZ0bPCByaz7wq573ylSe8Jlkzz1szyYlCmW2Vl0M9G58DNUM-miAZ74RlmErrI3q_bf2bmeEMT94L4D0-ujR2GAr5fEac_",
        "title": null,
        "width": 98
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 111,
        "rotate": null,
        "src": "https://lh6.googleusercontent.com/MDRhHpwIdCErJsYgABjkyykobV4YDn7RXe4bswEAH4LvcuRtXCN5kYtn62mcYtffzXBnoCZIa4QLVoY-_aDLMNoOkkCgc0EMpNxzMJn6Vrs-Um78m5tWXPsxp-Tk86kCXbheiaru",
        "title": null,
        "width": 99
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 24
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 2: Planning Your Experiment"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "For this checkpoint, you will define your hypothesis."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.bb74b06bcb0a3e5d36edb16b3325eb04f02b87de",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.7",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What Question will your group try to answer in this experiment?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "(Copy / paste from Checkpoint 1.)"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "When making elephant toothpaste, what will be the difference in the mass if we use twice the amount of hydrogen peroxide (236 mL versus 118 mL)?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.ea85d9a7a636d447d15e925e53b924543fea7e4e",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.8",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What"
          }, {
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": " is your group's hypothesis?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "We hypothesize that there will be more bubbles when we add twice the amount of hydrogen peroxide. In other experiments, when you add more of the active ingredient, the reaction is almost always bigger."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.bae547a74b6bd36b49d59832e8afb7ffab1ba47e",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.9",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Jalen         Course - Integrated Science 7         Project - Chemical Reactions"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 109,
        "rotate": null,
        "src": "https://lh4.googleusercontent.com/sd7A45VJC5SnPyeeQMX2Yh6T0yEyWWzu5qFEIpP1_i19_IS_v_7qDl_Wok3RRLJ2hTcr5YFgc7dXJW9b3Q4gDE8v4Xw_ucT_hoc3XA-IF8mW88bglGSFvC_ddedlkXJOiSIjaZPp",
        "title": null,
        "width": 94
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 112,
        "rotate": null,
        "src": "https://lh5.googleusercontent.com/Wo4RJuwW8_PR5BMiUUcGk7eLK4UGQPsvR60rJvSKmgTZ0bPCByaz7wq573ylSe8Jlkzz1szyYlCmW2Vl0M9G58DNUM-miAZ74RlmErrI3q_bf2bmeEMT94L4D0-ujR2GAr5fEac_",
        "title": null,
        "width": 98
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 111,
        "rotate": null,
        "src": "https://lh6.googleusercontent.com/MDRhHpwIdCErJsYgABjkyykobV4YDn7RXe4bswEAH4LvcuRtXCN5kYtn62mcYtffzXBnoCZIa4QLVoY-_aDLMNoOkkCgc0EMpNxzMJn6Vrs-Um78m5tWXPsxp-Tk86kCXbheiaru",
        "title": null,
        "width": 99
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 24
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 2: Planning Your Experiment"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "For this checkpoint, you will define your hypothesis."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.2cff70cb328833d1ea45218de709600ee2601866",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.10",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What Question will your group try to answer in this experiment?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "(Copy / paste from Checkpoint 1.)"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "When making elephant toothpaste, what will be the difference in the temperature if we use a 250 mL erlenmeyer flask or a 500 mL erlenmeyer flask?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.a4b096fc0b09678315ba5ebcab51b504b759736f",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.11",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What"
          }, {
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": " is your group's hypothesis?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arial"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "We hypothesize that the temperature will rise faster in the 500 mL erlenmeyer flask because there is more surface area and this will speed up the reaction and heat produced."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.0257dc846f755fe929f49558d0ba689b4013b454",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.12",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Ayden         Course - Integrated Science 7         Project - Chemical Reactions"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 122,
        "rotate": null,
        "src": "https://lh6.googleusercontent.com/Axi_tFGLfkGa1hAYh6L7uHrKUZEwr_wKKpWiIrZDlopIgU02fwVeGYOE6BCAV9RQZBq4LPNUAHL1maOzM7fvs5Z_JbqpxyCMKyRoKOzQrJBaUmZEgPVZXCBcTTMl8nTTPHTdPl_Q",
        "title": null,
        "width": 105
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 124,
        "rotate": null,
        "src": "https://lh3.googleusercontent.com/J9SSk1rWOdzkY63ChR-LdAJ3GBL0pZWQCmsNgVO4Crt-lR1dO-_pKn_76uquh9sc8W9Bz-dyI42AgG2I0EnBIIhBH7ag5LQt6sUOjsgMXgwpj19ZnnxlTW46Rf12t21L2TkDu7FP",
        "title": null,
        "width": 109
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 124,
        "rotate": null,
        "src": "https://lh6.googleusercontent.com/DacyM4Hv9SKHvE5ZLEf0d5to_AvA-gh1ko7F06NWolc80dcXVRl9plJFgXBJMHxum-Jp6-siX1FvzaRafiypHJ-70Agcu3Ek8fhO5B3LB2DngzXljyvZETbot7crqMcTu0UEY38s",
        "title": null,
        "width": 110
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 24
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 2: Planning Your Experiment"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "For this checkpoint, you will define your hypothesis."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.a9623803ea2fea6737e51fd5437e08f2debd3ab4",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.13",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What Question will your group try to answer in this experiment?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "(Copy / paste from Checkpoint 1.)"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "When we make elephant toothpaste, how will the bubbles be different if we use 236 mL versus 118 mL of hydrogen peroxide ?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.ff857141284d25c44a554fdbaeeb9050970f547b",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.14",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What"
          }, {
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": " is your group's hypothesis?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "We predict that the volume of bubbles will double when we add twice the amount of hydrogen peroxide."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.cb864d181c2b93a227d9022899769f3941db6a14",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.15",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Ali         Course - Integrated Science 7         Project - Chemical Reactions"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 122,
        "rotate": null,
        "src": "https://lh6.googleusercontent.com/Axi_tFGLfkGa1hAYh6L7uHrKUZEwr_wKKpWiIrZDlopIgU02fwVeGYOE6BCAV9RQZBq4LPNUAHL1maOzM7fvs5Z_JbqpxyCMKyRoKOzQrJBaUmZEgPVZXCBcTTMl8nTTPHTdPl_Q",
        "title": null,
        "width": 105
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 124,
        "rotate": null,
        "src": "https://lh3.googleusercontent.com/J9SSk1rWOdzkY63ChR-LdAJ3GBL0pZWQCmsNgVO4Crt-lR1dO-_pKn_76uquh9sc8W9Bz-dyI42AgG2I0EnBIIhBH7ag5LQt6sUOjsgMXgwpj19ZnnxlTW46Rf12t21L2TkDu7FP",
        "title": null,
        "width": 109
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 124,
        "rotate": null,
        "src": "https://lh6.googleusercontent.com/DacyM4Hv9SKHvE5ZLEf0d5to_AvA-gh1ko7F06NWolc80dcXVRl9plJFgXBJMHxum-Jp6-siX1FvzaRafiypHJ-70Agcu3Ek8fhO5B3LB2DngzXljyvZETbot7crqMcTu0UEY38s",
        "title": null,
        "width": 110
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 24
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 2: Planning Your Experiment"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "For this checkpoint, you will define your hypothesis."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.a454cabba71f6cb99580788d6e51a8114758b48a",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.16",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What Question will your group try to answer in this experiment?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "(Copy / paste from Checkpoint 1.)"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What is the reaction rate of hydrogen peroxide when the catalyst is yeast as compared to potassium iodide (and the control has no catalyst)?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.e510df9c2bea3c7645b6dfe45b1c100e61e28866",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.17",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 2,
          "rowspan": 1,
          "colwidth": null,
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "What"
          }, {
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "strong"
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": " is your group's hypothesis?"
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "125%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "I hypothesize that the control with the hydrogen peroxide on its own (and no catalyst) will take the longest to decompose (possibly several hours). Without a catalyst, the reaction will occur incredibly slowly. The second test with the yeast will take the second longest to decompose (possibly 5-15 minutes). The third test with the potassium iodide will decompose the fastest (under 5 minutes). "
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "125%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "125%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "em"
            }, {
              "type": "mark-font-size",
              "attrs": {
                "pt": 14
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Yeast is an organic compound and will therefore react more slowly than a purely chemical compound. Also, yeast might clump up and have less surface area; this would slow down the reaction. "
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.845d2ed3fe8cf412797b238c2437fe6ffce7c90a",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.18",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Zachary                Course - English 7         Project - From Story to Screen"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "id.f4hecsjcd653",
        "visible": true
      }
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Times New Roman"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3: Compare/Contrast Essay Outline"
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 122,
        "rotate": null,
        "src": "https://lh6.googleusercontent.com/5ur21Ku9KEPFKFBwoXx8uOAgBIHM995ANYnaGGOh9qwRyWjbq61o7GvUQa-04m49EaCHyVOu5-7ThJNF3mj0E0cGlTBNzacmVjv5dei8xBBHnyRYhbWVPk6emK7ctEEplodXZEKD",
        "title": null,
        "width": 214
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.3be6bafec82bb148df9b0acce2c3e245476834b6",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.19",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Complete an outline for your essay defending whether the movie or book is more powerful. "
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Main Claim:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "The film adaption of F. Scott Fitzgerald's "
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }, {
        "type": "underline"
      }],
      "text": "The Great Gatsby"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": " by Baz Luhrmann makes us like Gatsby and Daisy so much so that we can't see their flaws. This makes the movie less powerful."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #1:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "When I read the novel "
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }, {
        "type": "underline"
      }],
      "text": "The Great Gatsby"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": ", I choose how I want to see Gatsby and Daisy even though Nick tries to cover up their flaws."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #2:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "In the film, Daisy is too likeable even though she has no morals as a character."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #3:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "The film does a good job, however, of making me understand the luxury of Gatsby's lifestyle and parties."
    }]
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.1b1f1978dfd68ef89bcda463eddd4b1f037119ca",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.20",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Eliza                Course - English 7         Project - From Story to Screen"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Times New Roman"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3: Compare/Contrast Essay Outline"
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 134,
        "rotate": null,
        "src": "https://lh3.googleusercontent.com/38IdILPh09yzK3YlS6q-L_zedv6CMiSV11c4wSBxYJH64sBKarzuTUxehnf9QvJX-eE6KPYMXI2mRcymBMZO8OMJUhNIt5BjDYdZjRRlH_lUnK3WNfCpHmr3B5sfEe8Q07NGXO5B",
        "title": null,
        "width": 235
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.3be6bafec82bb148df9b0acce2c3e245476834b6",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.21",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Complete an outline for your essay defending whether the movie or book is more powerful. "
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Main Claim:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "The movie version of The Great Gatsby by Baz Luhrmann helped me better understand how Nick was the storyteller, the sequence of events in the story, and the importance of the symbols."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #1:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "The movie version made a change to show that Nick was in a mental hospital and writing the story out. This helped the audience to understand Nick's role in the story that wasn't clear in the novel."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #2:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "The novel is confusing because people are traveling from one place to another a lot and it's easy to lose track of the action."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #3:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "When we read a novel, the symbols are easy to miss, but that's not the case in film."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.167141b694b51171411480a63c60550ca3b125c2",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.22",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Li                Course - English 7         Project - From Story to Screen"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Times New Roman"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3: Compare/Contrast Essay Outline"
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 140,
        "rotate": null,
        "src": "https://lh4.googleusercontent.com/a4opfrF8Rb2UhQs7gSPpipcA5K8nVLE6GeMoJRwBjGXIm0FZ0PIvd_T_o0Lqsu_8TFTKPsZmC_J3Qzg-4yWay11pkA47qRIs93hpQHs_-Efa3nbDxT1ltj6jxCQfw5cXIh8nEUfF",
        "title": null,
        "width": 244
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.3be6bafec82bb148df9b0acce2c3e245476834b6",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.23",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Complete an outline for your essay defending whether the movie or book is more powerful. "
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Main Claim:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Books always make a better story experience than movies. When we compare the movie and book version of The Great Gatsby, the same is true."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #1:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "When we read the book, we can form our own opinions about characters and imagine what they are like on our own. We aren't forced to watch someone else's interpretation."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #2:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Movies always cut out pieces of the story for time and make the story less rich."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #3:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "We can't slow down and think for ourselves when we watch a movie. Books at least let us stop and think about the story more deeply."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.37681f73b9076bbf8d319fa77eeb8859b9cce8b2",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.24",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Jalen                 Course - English 7         Project - From Story to Screen"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Times New Roman"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3: Compare/Contrast Essay Outline"
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 149,
        "rotate": null,
        "src": "https://lh3.googleusercontent.com/j0064WUXYkdpv5ozW8Ad5N1ffEstlUmMcjD2TB8HvmQ1gzs64-bfozQZEpWB_8iR_ugGh-Ul6KLv__BwWUSfZRGBrn--ROrC_OtMs4Vg9_BQA9eK6h_mEJ7GGIbc-Jvbgm4aNJ-s",
        "title": null,
        "width": 261
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.3be6bafec82bb148df9b0acce2c3e245476834b6",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.25",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Complete an outline for your essay defending whether the movie or book is more powerful. "
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Main Claim:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Personally, reading books are better than watching movies due to how they explain stuff better and how they explain what some of the characters feel."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #1:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "144%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Books are better because they give you more background information and a better understanding of the characters."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #2:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "144%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Movies have a problem where they can't really show the thoughts of characters, even when those thoughts are very important to the storyline."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #3:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "144%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Movies, most of the time, are missing some of the important parts of the story that are  included in the book."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.85d95749dcc991ec16f5e301043830e790220ffd",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.26",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Ayden                 Course - English 7         Project - From Story to Screen"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Times New Roman"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3: Compare/Contrast Essay Outline"
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 139,
        "rotate": null,
        "src": "https://lh3.googleusercontent.com/gJhWQPeo6kRN3ueHjJueveviTsSzKfOx5PyDt6O1nGRsk5mi5fv5dvhNK7sq0Lh0yjEcaT8iOcXWnJNYT-YEgPFKaDMp91E6fsM49O14k5Q8vZipBJQQBHu9plSrcAYEaFXWu3qJ",
        "title": null,
        "width": 243
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.3be6bafec82bb148df9b0acce2c3e245476834b6",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.27",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Complete an outline for your essay defending whether the movie or book is more powerful. "
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Main Claim:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "The movie of The Great Gatsby is more powerful than the book because of the acting, costumes, and props."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #1:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "144%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "The movie version is superior because of the acting."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #2:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "144%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "The movie version is more powerful than the book because of the costumes and the props."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "144%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "144%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.eae0702854d05ea9f4080b98cfb33314c4f5abcf",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.28",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Ali                             Course - English 7         Project - From Story to Screen"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "166%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Times New Roman"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3: Compare/Contrast Essay Outline"
    }, {
      "type": "image",
      "attrs": {
        "align": null,
        "alt": null,
        "crop": null,
        "height": 120,
        "rotate": null,
        "src": "https://lh3.googleusercontent.com/gJhWQPeo6kRN3ueHjJueveviTsSzKfOx5PyDt6O1nGRsk5mi5fv5dvhNK7sq0Lh0yjEcaT8iOcXWnJNYT-YEgPFKaDMp91E6fsM49O14k5Q8vZipBJQQBHu9plSrcAYEaFXWu3qJ",
        "title": null,
        "width": 211
      },
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 11
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arial"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.3be6bafec82bb148df9b0acce2c3e245476834b6",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.29",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Complete an outline for your essay defending whether the movie or book is more powerful. "
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Main Claim:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Even though movies are visual spectacles, the book format will always be superior because of the amount of detail and description preserved, our imaginations will always create better scenes than directors, and written stories are limited in perspective the way movies are."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #1:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "While novelist put down the precise amount of detail, description, and information that we need to see as a reader to best understand the story, movies cut information and beautiful detail in order to be seen in a just one sitting."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #2:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Even though movie directors create visual scenes that capture our attention, we can imagine how a scene from a story might play out with various interpretations that can't be captured on screen."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "144%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "144%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Subclaim #3:"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 12
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Arimo"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "In stories, we get to hear character's thoughts and listen to whatever the narrator tells us. Movies typically don't reveal character thoughts or provide a narrator and therefore limit the perspectives we get to hear."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.c33e0d956f4acbcf8fc1d4e5ce49ee195bf37d0d",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.30",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Zachary      Course - Medieval History       Project - Feudal Honor Codes & Values"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "125%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "125%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "id.2eut15jp8ddz",
        "visible": true
      }
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 18
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3 - What purpose did social codes such as Bushido and Chivalry serve in Medieval Feudal systems?"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "180%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Directions:"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": " In the box below, answer the question above."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.22f02d03a0f7e2c0d6687f5dbb9fdba65cc883d0",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.31",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Knights and samurai followed their own social codes that benefitted both their lords and the common peasants."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Let's start with Knights. Knights helped lords, peasants, and even themselves. Knights followed an honor code called Chivalry, which said that they should have courage, fairness, mercy, and understanding. Since they worked for lords, the lords benefited because they collected taxes on lands, gave their peasants safety, and protected them (and the king). As I just mentioned, the peasants got protection from the knights. And, knights could change lords for more wealth or status. So, knights benefited because they could become more powerful."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Now, let's talk about samurai. Samurai also helped their lords, peasants, and their family (but not themselves). The honor code that samurai followed was called Bushido. Bushido required Samurai to have honor, duty, compassion, and selflessness. Like knights who worked for lords, Samurai worked for their masters called Daimyo. They didn't work for money and never changed masters. They were so loyal that when their masters died, they killed themselves. So, the Daimyo benefited because they had  Samurai to collect taxes, protect their lands and people, and be super loyal servants. The peasants got protection from the samurai. However, the samurai did not benefit financially. But if they died serving their master, then their family was cared for."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.0369d68539f7e9b1648552fd3892b62160babc5d",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.32",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Eliza      Course - Medieval History       Project - Feudal Honor Codes & Values"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "125%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 18
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3 - What purpose did social codes such as Bushido and Chivalry serve in Medieval Feudal systems?"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "180%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Directions:"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": " In the box below, answer the question above."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.9b1454e8884bf5d5f56084b067ff6cdf4f274b44",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.33",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "The purpose of social codes was to create strict rules for warriors: knights and samurai. These rules separated warriors and the average person and so that warriors would fit in that society. Knights and samurai would also protect their ruler. These social codes were very important in medieval times because they were the main protection for peasants but most of all their rulers."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Even though Chivalry was in Europe and Bushido was in Japan, knights and samurai  had very similar honor codes. Both knights and samurai had armor, swords, were paid in land, and had to be very honorable."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Both kinds of warriors had these social codes that worked for their society. They worked mostly for their lords to protect their lands and collect taxes. But the peasants also received protection."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Overall, Bushido and Chivalry were very important codes. And even though there were both types of warriors on opposite sides of the world, their social codes were very similar."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.9661c5b2fe9a01ed8b9bce861530998fca8ee45a",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.34",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Li      Course - Medieval History       Project - Feudal Honor Codes & Values"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "125%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 18
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3 - What purpose did social codes such as Bushido and Chivalry serve in Medieval Feudal systems?"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "180%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Directions:"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": " In the box below, answer the question above."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.a1b2356cba54d7df8c2bf6dee0dc23a1ab41d4ff",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.35",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Bushido is the social code for Samurai in Japan. Chivalry is the social code for Knights in Europe."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Both Bushido and Chivalry helped lords. Samurai and knights collected taxes for them."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Both Bushido and Chivalry helped peasants or common people. Samurai and knights protected the people because the social code told them to."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Both knights and samurai got jobs from this way of life. They worked and got land in return."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.7df42b07d523e9f190f1511beecd97b04b725e48",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.36",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Jalen      Course - Medieval History       Project - Feudal Honor Codes & Values"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "125%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 18
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3 - What purpose did social codes such as Bushido and Chivalry serve in Medieval Feudal systems?"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "180%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Directions:"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": " In the box below, answer the question above."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.80acff8765bbf1b34ab78bcfc92a4c9f6ddd90f2",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.37",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Bushido was the honor code for samurais in Japan. Chivalry was the honor code for knights in Europe. Even though these places were far away and didn't really talk to one another, both honor codes and warriors are pretty much the same. In both cases, they mainly helped the lords in the medieval feudal systems."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "The lords got all of the benefits from the knights and the samurai. The knights and the samurai collected taxes for their lords. They protected their lords. Samurai even thought it was honorable to kill themselves for their masters."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "144%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "The peasants also received protection, but it was probably to keep them in line. Knights and samurai protected workers on lands and the families. This is a good thing, but they also collected taxes too. So, the lords gained more than the people did."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.97abf2b57b100976d66f121514b3d8ee3d4f8f26",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.38",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Ayden      Course - Medieval History       Project - Feudal Honor Codes & Values"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "125%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 18
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3 - What purpose did social codes such as Bushido and Chivalry serve in Medieval Feudal systems?"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "180%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Directions:"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": " In the box below, answer the question above."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.98ba70fded75b50fbc050826a1ecf4ebad73089c",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.39",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Even though the samurai and knights came from different cultures, they did have some similarities. They both had social codes called Bushido and Chivalry."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "The samurai had to protect masters called daimyos while the knights protected lords. Because of Bushido and Chivalry, the samurai and knights both had to be respectful and honorable people. This means they protected other people in the land."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "The main reason why the had to have these set of rules was just to set an example so people would want to be like them."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "horizontal_rule",
    "attrs": {
      "pageBreak": true
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.02fad3f4e3af26e1d7e8ba97a89525227cb77371",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.40",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "120%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 11
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Student: Ali             Course - Medieval History       Project - Feudal Honor Codes & Values"
          }]
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "center",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "125%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 18
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Checkpoint 3 - What purpose did social codes such as Bushido and Chivalry serve in Medieval Feudal systems?"
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "180%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    },
    "content": [{
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "strong"
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": "Directions:"
    }, {
      "type": "text",
      "marks": [{
        "type": "mark-font-size",
        "attrs": {
          "pt": 14
        }
      }, {
        "type": "mark-font-type",
        "attrs": {
          "name": "Calibri"
        }
      }, {
        "type": "mark-text-color",
        "attrs": {
          "color": "#000000"
        }
      }],
      "text": " In the box below, answer the question above."
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": null,
      "color": null,
      "id": null,
      "indent": null,
      "lineSpacing": null,
      "paddingBottom": null,
      "paddingTop": null
    },
    "content": [{
      "type": "bookmark",
      "attrs": {
        "id": "t.eab8a5c6a55c0503b094adb34f155c8b0b5069fd",
        "visible": false
      }
    }, {
      "type": "bookmark",
      "attrs": {
        "id": "t.41",
        "visible": false
      }
    }]
  }, {
    "type": "table",
    "attrs": {
      "marginLeft": null
    },
    "content": [{
      "type": "table_row",
      "content": [{
        "type": "table_cell",
        "attrs": {
          "colspan": 1,
          "rowspan": 1,
          "colwidth": [620],
          "borderColor": "#000000",
          "background": null
        },
        "content": [{
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "In previous times in history, warriors or hunters or those in power would hurt those not in power because there were no social codes stopping them. But, both Bushido and Chivalry were social codes that helped protect the working class from the brutality of the warrior class."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "The samurai were fierce warriors that pledged allegiance to their masters: daimyos. They would carry swords, wear armor, and fight to the death thinking it was an honorable way to die. The social code of Bushido told samurais that they had to use their power to only help peasants or their masters. "
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Similarly, knights were also fearsome warriors that worked in service of lords. Like the samurai, they would also carry swords, wear armor, and essentially sell their fighting abilities for money. The social code of Chivalry was created by stories about how knights should behave in a kind, honorable, and genteel manner. "
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": "left",
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": "138%",
            "paddingBottom": "0pt",
            "paddingTop": "0pt"
          },
          "content": [{
            "type": "text",
            "marks": [{
              "type": "mark-font-size",
              "attrs": {
                "pt": 12
              }
            }, {
              "type": "mark-font-type",
              "attrs": {
                "name": "Arimo"
              }
            }, {
              "type": "mark-text-color",
              "attrs": {
                "color": "#000000"
              }
            }],
            "text": "Like Bushido, Chivalry limited how warriors could use their power and ended up helping the working class."
          }]
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }, {
          "type": "paragraph",
          "attrs": {
            "align": null,
            "color": null,
            "id": "",
            "indent": 0,
            "lineSpacing": null,
            "paddingBottom": "",
            "paddingTop": ""
          }
        }]
      }]
    }]
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "left",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }, {
    "type": "paragraph",
    "attrs": {
      "align": "right",
      "color": null,
      "id": "",
      "indent": 0,
      "lineSpacing": "138%",
      "paddingBottom": "0pt",
      "paddingTop": "0pt"
    }
  }]
}
  ))
end
